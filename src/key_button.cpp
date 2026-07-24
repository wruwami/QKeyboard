#include "qkeyboard/key_button.h"
#include "qkeyboard/keyboard_theme.h"

#include <QColor>
#include <QFontMetrics>
#include <QIcon>
#include <QResizeEvent>
#include <QStyle>

namespace qkw {

KeyButton::KeyButton(const QString &text, const QString &iconPath, QWidget *parent) : QPushButton(parent)
{
    setObjectName(text);
    if (!iconPath.isEmpty()) {
        setIcon(QIcon(iconPath));
    } else {
        setText(text);
    }
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(24, 24);
    setFlat(true);
}

bool KeyButton::isAccented() const
{
    return _accented;
}

void KeyButton::setAccented(bool accented)
{
    _accented = accented;
    setProperty("accented", accented);
    style()->unpolish(this);
    style()->polish(this);
}

void KeyButton::applyTheme(const KeyboardTheme &theme)
{
    const QColor keyBg = _accented ? theme.accentKeyColor() : theme.keyColor();
    setFont(theme.font());
    setStyleSheet(QStringLiteral("QPushButton { background-color: %1; color: %2; border-radius: %3px; border: none; }"
                                 "QPushButton:pressed { background-color: %4; }")
                      .arg(keyBg.name(QColor::HexArgb), theme.textColor().name(QColor::HexArgb))
                      .arg(theme.cornerRadius())
                      .arg(theme.keyPressedColor().name(QColor::HexArgb)));
    fitFontToButton();
    fitIconToButton();
}

void KeyButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    fitFontToButton();
    fitIconToButton();
}

void KeyButton::fitIconToButton()
{
    if (icon().isNull()) return;

    // Matches KeyboardKey.qml's icon sizing rule (40% of the smaller
    // dimension) so the two views render icons at comparable relative
    // sizes for the same button geometry.
    const int side = static_cast<int>(qMin(width(), height()) * 0.4);
    setIconSize(QSize(side, side));
}

void KeyButton::fitFontToButton()
{
    if (text().isEmpty()) return;

    const int targetHeight = rect().height() - 8;
    const int targetWidth = rect().width() - 8;
    if (targetHeight <= 0 || targetWidth <= 0) return;

    QFont current = font();
    const auto fitsAt = [&](qreal pointSize) {
        current.setPointSizeF(pointSize);
        const QRect bounds = QFontMetrics(current).boundingRect(text());
        return bounds.height() <= targetHeight && bounds.width() <= targetWidth;
    };

    qreal best = 6.0;
    for (qreal candidate = 6.0; candidate <= 48.0; candidate += 0.5) {
        if (fitsAt(candidate)) {
            best = candidate;
        } else if (candidate > 6.0) {
            break; // monotonic: once it stops fitting, larger sizes won't fit either
        }
    }
    current.setPointSizeF(best);
    setFont(current);
}

} // namespace qkw
