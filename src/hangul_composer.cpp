#include "qkeyboardwidget/hangul_composer.h"

namespace qkw {

namespace {

// Hangul Compatibility Jamo (U+3131..U+3163), in the fixed order used by the
// standard Unicode Hangul syllable composition formula:
//   syllable = 0xAC00 + (choseongIndex * 21 + jungseongIndex) * 28 + jongseongIndex
// This is the same block resources/layouts/ko.json's "char" keys emit one at a time.
const QString kChoseong = QStringLiteral("ㄱㄲㄴㄷㄸㄹㅁㅂ"
                                         "ㅃㅅㅆㅇㅈㅉㅊㅋ"
                                         "ㅌㅍㅎ");
const QString kJungseong = QStringLiteral("ㅏㅐㅑㅒㅓㅔㅕㅖ"
                                          "ㅗㅘㅙㅚㅛㅜㅝㅞ"
                                          "ㅟㅠㅡㅢㅣ");

int choseongIndexOf(QChar c)
{
    return kChoseong.indexOf(c);
}

int jungseongIndexOf(QChar c)
{
    return kJungseong.indexOf(c);
}

// Single (non-compound) consonant -> jongseong index, or -1 if that
// consonant can never be a syllable-final (ㄸ/ㅃ/ㅉ have no batchim form).
int choseongToSingleJongseong(int choseongIndex)
{
    switch (choseongIndex) {
        case 0: return 1;   // ㄱ -> ㄱ
        case 1: return 2;   // ㄲ -> ㄲ
        case 2: return 4;   // ㄴ -> ㄴ
        case 3: return 7;   // ㄷ -> ㄷ
        case 5: return 8;   // ㄹ -> ㄹ
        case 6: return 16;  // ㅁ -> ㅁ
        case 7: return 17;  // ㅂ -> ㅂ
        case 9: return 19;  // ㅅ -> ㅅ
        case 10: return 20; // ㅆ -> ㅆ
        case 11: return 21; // ㅇ -> ㅇ
        case 12: return 22; // ㅈ -> ㅈ
        case 14: return 23; // ㅊ -> ㅊ
        case 15: return 24; // ㅋ -> ㅋ
        case 16: return 25; // ㅌ -> ㅌ
        case 17: return 26; // ㅍ -> ㅍ
        case 18: return 27; // ㅎ -> ㅎ
        default: return -1; // ㄸ, ㅃ, ㅉ
    }
}

// Single jongseong index -> the choseong index it becomes when pulled into
// the next syllable (only defined for non-compound jongseong indices).
int singleJongseongToChoseong(int jongseongIndex)
{
    switch (jongseongIndex) {
        case 1: return 0;   // ㄱ -> ㄱ
        case 2: return 1;   // ㄲ -> ㄲ
        case 4: return 2;   // ㄴ -> ㄴ
        case 7: return 3;   // ㄷ -> ㄷ
        case 8: return 5;   // ㄹ -> ㄹ
        case 16: return 6;  // ㅁ -> ㅁ
        case 17: return 7;  // ㅂ -> ㅂ
        case 19: return 9;  // ㅅ -> ㅅ
        case 20: return 10; // ㅆ -> ㅆ
        case 21: return 11; // ㅇ -> ㅇ
        case 22: return 12; // ㅈ -> ㅈ
        case 23: return 14; // ㅊ -> ㅊ
        case 24: return 15; // ㅋ -> ㅋ
        case 25: return 16; // ㅌ -> ㅌ
        case 26: return 17; // ㅍ -> ㅍ
        case 27: return 18; // ㅎ -> ㅎ
        default: return -1;
    }
}

// Combines an existing (single) jongseong with a newly-typed consonant into
// a compound jongseong, or returns -1 if that pair doesn't combine.
int combineJongseong(int baseJongseongIndex, int addedChoseongIndex)
{
    switch (baseJongseongIndex) {
        case 1: return addedChoseongIndex == 9 ? 3 : -1; // ㄱ+ㅅ -> ㄳ
        case 4:                                          // ㄴ+...
            if (addedChoseongIndex == 12) return 5;      // ㄴ+ㅈ -> ㄵ
            if (addedChoseongIndex == 18) return 6;      // ㄴ+ㅎ -> ㄶ
            return -1;
        case 8: // ㄹ+...
            switch (addedChoseongIndex) {
                case 0: return 9;   // ㄹ+ㄱ -> ㄺ
                case 6: return 10;  // ㄹ+ㅁ -> ㄻ
                case 7: return 11;  // ㄹ+ㅂ -> ㄼ
                case 9: return 12;  // ㄹ+ㅅ -> ㄽ
                case 16: return 13; // ㄹ+ㅌ -> ㄾ
                case 17: return 14; // ㄹ+ㅍ -> ㄿ
                case 18: return 15; // ㄹ+ㅎ -> ㅀ
                default: return -1;
            }
        case 17: return addedChoseongIndex == 9 ? 18 : -1; // ㅂ+ㅅ -> ㅄ
        default: return -1;
    }
}

// Splits a compound jongseong back into the part that stays as this
// syllable's final and the consonant that gets pulled into the next
// syllable's initial. Returns {-1, -1} if `jongseongIndex` isn't compound.
struct JongseongSplit
{
    int remainingJongseong;
    int pulledChoseong;
};

JongseongSplit decomposeCompoundJongseong(int jongseongIndex)
{
    switch (jongseongIndex) {
        case 3: return {1, 9};   // ㄳ -> ㄱ + ㅅ
        case 5: return {4, 12};  // ㄵ -> ㄴ + ㅈ
        case 6: return {4, 18};  // ㄶ -> ㄴ + ㅎ
        case 9: return {8, 0};   // ㄺ -> ㄹ + ㄱ
        case 10: return {8, 6};  // ㄻ -> ㄹ + ㅁ
        case 11: return {8, 7};  // ㄼ -> ㄹ + ㅂ
        case 12: return {8, 9};  // ㄽ -> ㄹ + ㅅ
        case 13: return {8, 16}; // ㄾ -> ㄹ + ㅌ
        case 14: return {8, 17}; // ㄿ -> ㄹ + ㅍ
        case 15: return {8, 18}; // ㅀ -> ㄹ + ㅎ
        case 18: return {17, 9}; // ㅄ -> ㅂ + ㅅ
        default: return {-1, -1};
    }
}

bool isCompoundJongseong(int jongseongIndex)
{
    return decomposeCompoundJongseong(jongseongIndex).remainingJongseong != -1;
}

// Combines an existing (single) jungseong with a newly-typed vowel into a
// compound jungseong, or returns -1 if that pair doesn't combine.
int combineJungseong(int baseJungseongIndex, int addedJungseongIndex)
{
    switch (baseJungseongIndex) {
        case 8: // ㅗ+...
            switch (addedJungseongIndex) {
                case 0: return 9;   // ㅗ+ㅏ -> ㅘ
                case 1: return 10;  // ㅗ+ㅐ -> ㅙ
                case 20: return 11; // ㅗ+ㅣ -> ㅚ
                default: return -1;
            }
        case 13: // ㅜ+...
            switch (addedJungseongIndex) {
                case 4: return 14;  // ㅜ+ㅓ -> ㅝ
                case 5: return 15;  // ㅜ+ㅔ -> ㅞ
                case 20: return 16; // ㅜ+ㅣ -> ㅟ
                default: return -1;
            }
        case 18: return addedJungseongIndex == 20 ? 19 : -1; // ㅡ+ㅣ -> ㅢ
        default: return -1;
    }
}

// Single (non-compound) jungseong index this compound vowel decomposes down
// to when backspacing one keystroke's worth. -1 if not compound.
int decomposeCompoundJungseong(int jungseongIndex)
{
    switch (jungseongIndex) {
        case 9:
        case 10:
        case 11: return 8; // ㅘ/ㅙ/ㅚ -> ㅗ
        case 14:
        case 15:
        case 16: return 13; // ㅝ/ㅞ/ㅟ -> ㅜ
        case 19: return 18; // ㅢ -> ㅡ
        default: return -1;
    }
}

} // namespace

HangulComposer::HangulComposer(QObject *parent) : AbstractComposer(parent), _cho(-1), _jung(-1), _jong(0)
{
}

QString HangulComposer::composeCurrent() const
{
    if (_cho == -1) return {};
    if (_jung == -1) return kChoseong.mid(_cho, 1);
    const int codepoint = 0xAC00 + (_cho * 21 + _jung) * 28 + _jong;
    return QString(QChar(codepoint));
}

void HangulComposer::startNewSyllable(int choseongIndex)
{
    _cho = choseongIndex;
    _jung = -1;
    _jong = 0;
}

bool HangulComposer::feed(const QString &text)
{
    if (text.size() != 1) {
        commit();
        return false;
    }
    const QChar c = text.at(0);
    const int consIdx = choseongIndexOf(c);
    const int vowIdx = jungseongIndexOf(c);

    if (consIdx == -1 && vowIdx == -1) {
        commit();
        return false;
    }

    if (consIdx != -1) {
        if (_cho == -1) {
            startNewSyllable(consIdx);
            emit syllableReady(composeCurrent(), false);
            return true;
        }
        if (_jung == -1) {
            // Two choseong in a row (e.g. ㄱㄴ) can't combine into one
            // initial; the first stands alone and the second starts fresh.
            startNewSyllable(consIdx);
            emit syllableReady(composeCurrent(), false);
            return true;
        }
        if (_jong == 0) {
            const int candidate = choseongToSingleJongseong(consIdx);
            if (candidate == -1) {
                startNewSyllable(consIdx);
                emit syllableReady(composeCurrent(), false);
                return true;
            }
            _jong = candidate;
            emit syllableReady(composeCurrent(), true);
            return true;
        }
        const int combined = combineJongseong(_jong, consIdx);
        if (combined != -1) {
            _jong = combined;
            emit syllableReady(composeCurrent(), true);
            return true;
        }
        startNewSyllable(consIdx);
        emit syllableReady(composeCurrent(), false);
        return true;
    }

    // vowIdx != -1
    if (_cho == -1) {
        // No pending initial to attach to (e.g. layout/user error); show the
        // vowel standalone rather than silently dropping the keystroke.
        emit syllableReady(text, false);
        return true;
    }
    if (_jung == -1) {
        _jung = vowIdx;
        _jong = 0;
        emit syllableReady(composeCurrent(), true);
        return true;
    }
    if (_jong == 0) {
        const int combined = combineJungseong(_jung, vowIdx);
        if (combined != -1) {
            _jung = combined;
            emit syllableReady(composeCurrent(), true);
            return true;
        }
        // Two vowels that don't combine: the pending syllable has no
        // initial to offer this one, so it can't continue composing either.
        commit();
        emit syllableReady(text, false);
        return true;
    }

    // cho+jung+jong all set: the jongseong belongs to the next syllable's
    // initial instead (standard "pull-back" rule), unless it's a compound
    // final, in which case only its second consonant is pulled.
    int remainingJong;
    int pulledCho;
    if (isCompoundJongseong(_jong)) {
        const JongseongSplit split = decomposeCompoundJongseong(_jong);
        remainingJong = split.remainingJongseong;
        pulledCho = split.pulledChoseong;
    } else {
        remainingJong = 0;
        pulledCho = singleJongseongToChoseong(_jong);
    }

    _jong = remainingJong;
    emit syllableReady(composeCurrent(), true);

    startNewSyllable(pulledCho);
    _jung = vowIdx;
    _jong = 0;
    emit syllableReady(composeCurrent(), false);
    return true;
}

bool HangulComposer::backspace()
{
    if (_jong != 0) {
        if (isCompoundJongseong(_jong)) {
            _jong = decomposeCompoundJongseong(_jong).remainingJongseong;
        } else {
            _jong = 0;
        }
        emit syllableReady(composeCurrent(), true);
        return true;
    }
    if (_jung != -1) {
        _jung = decomposeCompoundJungseong(_jung);
        emit syllableReady(composeCurrent(), true);
        return true;
    }
    if (_cho != -1) {
        _cho = -1;
        emit syllableCleared();
        return true;
    }
    return false;
}

void HangulComposer::commit()
{
    _cho = -1;
    _jung = -1;
    _jong = 0;
}

void HangulComposer::reset()
{
    commit();
}

bool HangulComposer::isComposing() const
{
    return _cho != -1;
}

} // namespace qkw
