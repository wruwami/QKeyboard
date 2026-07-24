#include "qkeyboard/kana_composer.h"

#include <QMap>

namespace qkw {

namespace {

const QMap<QString, QString> &dakutenMap()
{
    static const QMap<QString, QString> map = {
        {QStringLiteral("か"), QStringLiteral("が")}, {QStringLiteral("き"), QStringLiteral("ぎ")},
        {QStringLiteral("く"), QStringLiteral("ぐ")}, {QStringLiteral("け"), QStringLiteral("げ")},
        {QStringLiteral("こ"), QStringLiteral("ご")}, {QStringLiteral("さ"), QStringLiteral("ざ")},
        {QStringLiteral("し"), QStringLiteral("じ")}, {QStringLiteral("す"), QStringLiteral("ず")},
        {QStringLiteral("せ"), QStringLiteral("ぜ")}, {QStringLiteral("そ"), QStringLiteral("ぞ")},
        {QStringLiteral("た"), QStringLiteral("だ")}, {QStringLiteral("ち"), QStringLiteral("ぢ")},
        {QStringLiteral("つ"), QStringLiteral("づ")}, {QStringLiteral("て"), QStringLiteral("で")},
        {QStringLiteral("と"), QStringLiteral("ど")}, {QStringLiteral("は"), QStringLiteral("ば")},
        {QStringLiteral("ひ"), QStringLiteral("び")}, {QStringLiteral("ふ"), QStringLiteral("ぶ")},
        {QStringLiteral("へ"), QStringLiteral("べ")}, {QStringLiteral("ほ"), QStringLiteral("ぼ")},
        {QStringLiteral("カ"), QStringLiteral("ガ")}, {QStringLiteral("キ"), QStringLiteral("ギ")},
        {QStringLiteral("ク"), QStringLiteral("グ")}, {QStringLiteral("ケ"), QStringLiteral("ゲ")},
        {QStringLiteral("コ"), QStringLiteral("ゴ")}, {QStringLiteral("サ"), QStringLiteral("ザ")},
        {QStringLiteral("シ"), QStringLiteral("ジ")}, {QStringLiteral("ス"), QStringLiteral("ズ")},
        {QStringLiteral("セ"), QStringLiteral("ゼ")}, {QStringLiteral("ソ"), QStringLiteral("ゾ")},
        {QStringLiteral("タ"), QStringLiteral("ダ")}, {QStringLiteral("チ"), QStringLiteral("ヂ")},
        {QStringLiteral("ツ"), QStringLiteral("ヅ")}, {QStringLiteral("テ"), QStringLiteral("デ")},
        {QStringLiteral("ト"), QStringLiteral("ド")}, {QStringLiteral("ハ"), QStringLiteral("バ")},
        {QStringLiteral("ヒ"), QStringLiteral("ビ")}, {QStringLiteral("フ"), QStringLiteral("ブ")},
        {QStringLiteral("ヘ"), QStringLiteral("ベ")}, {QStringLiteral("ホ"), QStringLiteral("ボ")},
    };
    return map;
}

const QMap<QString, QString> &handakutenMap()
{
    static const QMap<QString, QString> map = {
        {QStringLiteral("は"), QStringLiteral("ぱ")}, {QStringLiteral("ひ"), QStringLiteral("ぴ")},
        {QStringLiteral("ふ"), QStringLiteral("ぷ")}, {QStringLiteral("へ"), QStringLiteral("ぺ")},
        {QStringLiteral("ほ"), QStringLiteral("ぽ")}, {QStringLiteral("ハ"), QStringLiteral("パ")},
        {QStringLiteral("ヒ"), QStringLiteral("ピ")}, {QStringLiteral("フ"), QStringLiteral("プ")},
        {QStringLiteral("ヘ"), QStringLiteral("ペ")}, {QStringLiteral("ホ"), QStringLiteral("ポ")},
    };
    return map;
}

const QMap<QString, QString> &smallKanaMap()
{
    static const QMap<QString, QString> map = {
        {QStringLiteral("あ"), QStringLiteral("ぁ")}, {QStringLiteral("い"), QStringLiteral("ぃ")},
        {QStringLiteral("う"), QStringLiteral("ぅ")}, {QStringLiteral("え"), QStringLiteral("ぇ")},
        {QStringLiteral("お"), QStringLiteral("ぉ")}, {QStringLiteral("つ"), QStringLiteral("っ")},
        {QStringLiteral("や"), QStringLiteral("ゃ")}, {QStringLiteral("ゆ"), QStringLiteral("ゅ")},
        {QStringLiteral("よ"), QStringLiteral("ょ")}, {QStringLiteral("わ"), QStringLiteral("ゎ")},
        {QStringLiteral("ぁ"), QStringLiteral("あ")}, {QStringLiteral("ぃ"), QStringLiteral("い")},
        {QStringLiteral("ぅ"), QStringLiteral("う")}, {QStringLiteral("ぇ"), QStringLiteral("え")},
        {QStringLiteral("ぉ"), QStringLiteral("お")}, {QStringLiteral("っ"), QStringLiteral("つ")},
        {QStringLiteral("ゃ"), QStringLiteral("や")}, {QStringLiteral("ゅ"), QStringLiteral("ゆ")},
        {QStringLiteral("ょ"), QStringLiteral("よ")}, {QStringLiteral("ゎ"), QStringLiteral("わ")},
    };
    return map;
}

// Finds the key whose value is `value` in `map` (dakutenMap()/handakutenMap()/
// smallKanaMap() are all small enough that a linear scan is fine, and none of
// them are large or hot-path enough to warrant a second, value-to-key map
// just for this). Returns an empty string if no key maps to `value` - used by
// KanaComposer::backspace() to undo whichever modifier (if any) produced the
// current kana.
QString reverseLookup(const QMap<QString, QString> &map, const QString &value)
{
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        if (it.value() == value) return it.key();
    }
    return QString();
}

} // namespace

KanaComposer::KanaComposer(QObject *parent) : AbstractComposer(parent)
{
}

bool KanaComposer::feed(const QString &text)
{
    if (text.isEmpty()) return false;

    // Handle dakuten modifier "゛" or "″"
    if (text == QStringLiteral("゛") || text == QStringLiteral("″")) {
        if (!isComposing()) return false;
        if (dakutenMap().contains(_currentKana)) {
            _currentKana = dakutenMap().value(_currentKana);
            _lastModifier = LastModifier::Dakuten;
            emit syllableReady(_currentKana, true);
            return true;
        }
        return false;
    }

    // Handle handakuten modifier "゜" or "°"
    if (text == QStringLiteral("゜") || text == QStringLiteral("°")) {
        if (!isComposing()) return false;
        if (handakutenMap().contains(_currentKana)) {
            _currentKana = handakutenMap().value(_currentKana);
            _lastModifier = LastModifier::Handakuten;
            emit syllableReady(_currentKana, true);
            return true;
        }
        return false;
    }

    // Handle small/large kana toggle key "小" or "小/大"
    if (text == QStringLiteral("小") || text == QStringLiteral("小/大")) {
        if (!isComposing()) return false;
        if (smallKanaMap().contains(_currentKana)) {
            _currentKana = smallKanaMap().value(_currentKana);
            _lastModifier = LastModifier::SmallKana;
            emit syllableReady(_currentKana, true);
            return true;
        }
        return false;
    }

    // Standard new kana entry
    _currentKana = text;
    _lastModifier = LastModifier::None;
    emit syllableReady(_currentKana, false);
    return true;
}

bool KanaComposer::backspace()
{
    if (!isComposing()) return false;

    // Undo whichever modifier produced the current kana, tracked explicitly
    // in _lastModifier rather than guessed from map membership - see the
    // comment on _lastModifier's declaration for why that guess is
    // unreliable specifically for smallKanaMap().
    QString original;
    switch (_lastModifier) {
        case LastModifier::Dakuten: original = reverseLookup(dakutenMap(), _currentKana); break;
        case LastModifier::Handakuten: original = reverseLookup(handakutenMap(), _currentKana); break;
        case LastModifier::SmallKana: original = reverseLookup(smallKanaMap(), _currentKana); break;
        case LastModifier::None: break;
    }

    if (!original.isEmpty()) {
        _currentKana = original;
        _lastModifier = LastModifier::None;
        emit syllableReady(_currentKana, true);
        return true;
    }

    // Clear lone kana
    _currentKana.clear();
    _lastModifier = LastModifier::None;
    emit syllableCleared();
    return true;
}

void KanaComposer::reset()
{
    if (!_currentKana.isEmpty()) {
        _currentKana.clear();
        _lastModifier = LastModifier::None;
        emit syllableCleared();
    }
}

bool KanaComposer::isComposing() const
{
    return !_currentKana.isEmpty();
}

} // namespace qkw
