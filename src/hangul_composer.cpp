#include "qkeyboardwidget/hangul_composer.h"
#include "qkeyboardwidget/keyboard_controller.h"

namespace qkw {

namespace {

static const QChar kChoseongs[] = {
    QChar(0x3131), QChar(0x3132), QChar(0x3134), QChar(0x3137), QChar(0x3138),
    QChar(0x3139), QChar(0x3141), QChar(0x3142), QChar(0x3143), QChar(0x3145),
    QChar(0x3146), QChar(0x3147), QChar(0x3148), QChar(0x3149), QChar(0x314A),
    QChar(0x314B), QChar(0x314C), QChar(0x314D), QChar(0x314E)
};

static const QChar kJungseongs[] = {
    QChar(0x314F), QChar(0x3150), QChar(0x3151), QChar(0x3152), QChar(0x3153),
    QChar(0x3154), QChar(0x3155), QChar(0x3156), QChar(0x3157), QChar(0x3158),
    QChar(0x3159), QChar(0x315A), QChar(0x315B), QChar(0x315C), QChar(0x315D),
    QChar(0x315E), QChar(0x315F), QChar(0x3160), QChar(0x3161), QChar(0x3162),
    QChar(0x3163)
};

static const QChar kJongseongs[] = {
    QChar(0),      QChar(0x3131), QChar(0x3132), QChar(0x3133), QChar(0x3134),
    QChar(0x3135), QChar(0x3136), QChar(0x3137), QChar(0x3139), QChar(0x313A),
    QChar(0x313B), QChar(0x313C), QChar(0x313D), QChar(0x313E), QChar(0x313F),
    QChar(0x3140), QChar(0x3141), QChar(0x3142), QChar(0x3144), QChar(0x3145),
    QChar(0x3146), QChar(0x3147), QChar(0x3148), QChar(0x314A), QChar(0x314B),
    QChar(0x314C), QChar(0x314D), QChar(0x314E)
};

} // namespace

HangulComposer::HangulComposer(QObject *parent)
    : QObject(parent)
{
}

HangulComposer::HangulComposer(KeyboardController *controller, QObject *parent)
    : QObject(parent)
{
    setController(controller);
}

HangulComposer::~HangulComposer() = default;

KeyboardController *HangulComposer::controller() const
{
    return _controller;
}

void HangulComposer::setController(KeyboardController *controller)
{
    if (_controller == controller) return;

    if (_controller) {
        disconnect(_controller, &KeyboardController::characterEntered, this, &HangulComposer::handleCharacterEntered);
        disconnect(_controller, &KeyboardController::backspaceRequested, this, &HangulComposer::handleBackspaceRequested);
        disconnect(_controller, &KeyboardController::layoutChanged, this, &HangulComposer::handleLayoutChanged);
    }

    _controller = controller;

    if (_controller) {
        connect(_controller, &KeyboardController::characterEntered, this, &HangulComposer::handleCharacterEntered);
        connect(_controller, &KeyboardController::backspaceRequested, this, &HangulComposer::handleBackspaceRequested);
        connect(_controller, &KeyboardController::layoutChanged, this, &HangulComposer::handleLayoutChanged);
    }

    handleLayoutChanged();
    emit controllerChanged();
}

bool HangulComposer::isComposing() const
{
    return !_current.isEmpty();
}

QString HangulComposer::preeditText() const
{
    return syllableToString(_current);
}

void HangulComposer::commit()
{
    if (!_current.isEmpty()) {
        _current.clear();
        emit compositionChanged();
    }
}

void HangulComposer::handleCharacterEntered(const QString &text)
{
    if (!_isKorean || text.length() != 1) {
        commit();
        emit characterEntered(text);
        return;
    }

    QChar c = text.at(0);
    if (!isConsonant(c) && !isVowel(c)) {
        commit();
        emit characterEntered(text);
        return;
    }

    QString oldPreedit = syllableToString(_current);
    QString commitText = processKey(c);
    QString newPreedit = syllableToString(_current);

    // Sequence to update text field:
    // 1. Backspace old preedit
    for (int i = 0; i < oldPreedit.length(); ++i) {
        emit backspaceRequested();
    }
    // 2. Insert committed text (if any)
    if (!commitText.isEmpty()) {
        emit characterEntered(commitText);
    }
    // 3. Insert new preedit (if any)
    if (!newPreedit.isEmpty()) {
        emit characterEntered(newPreedit);
    }

    emit compositionChanged();
}

void HangulComposer::handleBackspaceRequested()
{
    if (!_isKorean || _current.isEmpty()) {
        emit backspaceRequested();
        return;
    }

    QString oldPreedit = syllableToString(_current);

    // Decompose the current syllable step-by-step
    if (!_current.jong2.isNull()) {
        _current.jong2 = QChar();
        _current.jong = _current.jong1;
    } else if (!_current.jong.isNull()) {
        _current.jong = QChar();
        _current.jong1 = QChar();
    } else if (!_current.jung2.isNull()) {
        _current.jung2 = QChar();
        _current.jung = _current.jung1;
    } else if (!_current.jung.isNull()) {
        _current.jung = QChar();
        _current.jung1 = QChar();
    } else if (!_current.cho.isNull()) {
        _current.cho = QChar();
    }

    QString newPreedit = syllableToString(_current);

    // Update text field: backspace old preedit and insert new preedit
    for (int i = 0; i < oldPreedit.length(); ++i) {
        emit backspaceRequested();
    }
    if (!newPreedit.isEmpty()) {
        emit characterEntered(newPreedit);
    }

    emit compositionChanged();
}

void HangulComposer::handleLayoutChanged()
{
    commit();
    bool wasKorean = _isKorean;
    _isKorean = (_controller && _controller->locale() == QStringLiteral("ko"));
    if (wasKorean != _isKorean) {
        emit compositionChanged();
    }
}

bool HangulComposer::isConsonant(QChar c) const
{
    ushort val = c.unicode();
    return val >= 0x3131 && val <= 0x314E;
}

bool HangulComposer::isVowel(QChar c) const
{
    ushort val = c.unicode();
    return val >= 0x314F && val <= 0x3163;
}

QChar HangulComposer::combineVowels(QChar v1, QChar v2) const
{
    if (v1 == 0x3157) { // ㅗ
        if (v2 == 0x314F) return QChar(0x3158); // ㅏ -> ㅘ
        if (v2 == 0x3150) return QChar(0x3159); // ㅐ -> ㅙ
        if (v2 == 0x3163) return QChar(0x315A); // ㅣ -> ㅚ
    } else if (v1 == 0x315C) { // ㅜ
        if (v2 == 0x3153) return QChar(0x315D); // ㅓ -> ㅝ
        if (v2 == 0x3154) return QChar(0x315E); // ㅔ -> ㅞ
        if (v2 == 0x3163) return QChar(0x315F); // ㅣ -> ㅟ
    } else if (v1 == 0x3161) { // ㅡ
        if (v2 == 0x3163) return QChar(0x3162); // ㅣ -> ㅢ
    }
    return QChar(0);
}

QChar HangulComposer::combineConsonants(QChar c1, QChar c2) const
{
    if (c1 == 0x3131) { // ㄱ
        if (c2 == 0x3145) return QChar(0x3133); // ㅅ -> ㄳ
    } else if (c1 == 0x3134) { // ㄴ
        if (c2 == 0x3148) return QChar(0x3135); // ㅈ -> ㄵ
        if (c2 == 0x314E) return QChar(0x3136); // ㅎ -> ㄶ
    } else if (c1 == 0x3139) { // ㄹ
        if (c2 == 0x3131) return QChar(0x313A); // ㄱ -> ㄺ
        if (c2 == 0x3141) return QChar(0x313B); // ㅁ -> ㄻ
        if (c2 == 0x3142) return QChar(0x313C); // ㅂ -> ㄼ
        if (c2 == 0x3145) return QChar(0x313D); // ㅅ -> ㄽ
        if (c2 == 0x314C) return QChar(0x313E); // ㅌ -> ㄾ
        if (c2 == 0x314D) return QChar(0x313F); // ㅍ -> ㄿ
        if (c2 == 0x314E) return QChar(0x3140); // ㅎ -> ㅀ
    } else if (c1 == 0x3142) { // ㅂ
        if (c2 == 0x3145) return QChar(0x3144); // ㅅ -> ㅄ
    }
    return QChar(0);
}

int HangulComposer::getChoseongIndex(QChar c) const
{
    for (int i = 0; i < 19; ++i) {
        if (kChoseongs[i] == c) return i;
    }
    return -1;
}

int HangulComposer::getJungseongIndex(QChar c) const
{
    for (int i = 0; i < 21; ++i) {
        if (kJungseongs[i] == c) return i;
    }
    return -1;
}

int HangulComposer::getJongseongIndex(QChar c) const
{
    for (int i = 0; i < 28; ++i) {
        if (kJongseongs[i] == c) return i;
    }
    return -1;
}

QString HangulComposer::syllableToString(const HangulSyllable &s) const
{
    if (s.isEmpty()) {
        return QString();
    }
    if (!s.cho.isNull() && s.jung.isNull()) {
        return QString(s.cho);
    }
    if (s.cho.isNull() && !s.jung.isNull()) {
        return QString(s.jung);
    }

    int l = getChoseongIndex(s.cho);
    int v = getJungseongIndex(s.jung);
    int t = getJongseongIndex(s.jong);

    if (l >= 0 && v >= 0 && t >= 0) {
        char32_t unicode = 0xAC00 + (l * 21 * 28) + (v * 28) + t;
        return QString::fromUcs4(&unicode, 1);
    }

    QString result;
    if (!s.cho.isNull()) result += s.cho;
    if (!s.jung.isNull()) result += s.jung;
    if (!s.jong.isNull()) result += s.jong;
    return result;
}

QString HangulComposer::processKey(QChar c)
{
    QString commitText;

    if (_current.isEmpty()) {
        if (isConsonant(c)) {
            _current.cho = c;
        } else {
            _current.jung = c;
            _current.jung1 = c;
        }
        return commitText;
    }

    // Case 2: has cho but no jung
    if (!_current.cho.isNull() && _current.jung.isNull()) {
        if (isConsonant(c)) {
            commitText = syllableToString(_current);
            _current.clear();
            _current.cho = c;
        } else {
            _current.jung = c;
            _current.jung1 = c;
        }
        return commitText;
    }

    // Case 3: has cho and jung but no jong
    if (!_current.cho.isNull() && !_current.jung.isNull() && _current.jong.isNull()) {
        if (isConsonant(c)) {
            if (getJongseongIndex(c) > 0) {
                _current.jong = c;
                _current.jong1 = c;
            } else {
                commitText = syllableToString(_current);
                _current.clear();
                _current.cho = c;
            }
        } else {
            QChar combined = combineVowels(_current.jung, c);
            if (!combined.isNull()) {
                _current.jung = combined;
                _current.jung2 = c;
            } else {
                commitText = syllableToString(_current);
                _current.clear();
                _current.jung = c;
                _current.jung1 = c;
            }
        }
        return commitText;
    }

    // Case 4: has cho, jung, and jong
    if (!_current.cho.isNull() && !_current.jung.isNull() && !_current.jong.isNull()) {
        if (isConsonant(c)) {
            if (_current.jong2.isNull()) {
                QChar combined = combineConsonants(_current.jong, c);
                if (!combined.isNull()) {
                    _current.jong = combined;
                    _current.jong2 = c;
                } else {
                    commitText = syllableToString(_current);
                    _current.clear();
                    _current.cho = c;
                }
            } else {
                commitText = syllableToString(_current);
                _current.clear();
                _current.cho = c;
            }
        } else {
            // Syllable splitting
            if (!_current.jong2.isNull()) {
                QChar movePart = _current.jong2;
                _current.jong = _current.jong1;
                _current.jong2 = QChar();
                commitText = syllableToString(_current);

                _current.clear();
                _current.cho = movePart;
                _current.jung = c;
                _current.jung1 = c;
            } else {
                QChar movePart = _current.jong;
                _current.jong = QChar();
                _current.jong1 = QChar();
                commitText = syllableToString(_current);

                _current.clear();
                _current.cho = movePart;
                _current.jung = c;
                _current.jung1 = c;
            }
        }
        return commitText;
    }

    // Case 5: no cho but has jung
    if (_current.cho.isNull() && !_current.jung.isNull()) {
        if (isConsonant(c)) {
            commitText = syllableToString(_current);
            _current.clear();
            _current.cho = c;
        } else {
            QChar combined = combineVowels(_current.jung, c);
            if (!combined.isNull()) {
                _current.jung = combined;
                _current.jung2 = c;
            } else {
                commitText = syllableToString(_current);
                _current.clear();
                _current.jung = c;
                _current.jung1 = c;
            }
        }
        return commitText;
    }

    return commitText;
}

} // namespace qkw
