#include "qkeyboardwidget/key_button.h"
#include "qkeyboardwidget/keyboard_theme.h"

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
}

void KeyButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    fitFontToButton();
}

void KeyButton::fitFontToButton()
{
    if (text().isEmpty()) return;

    QFont current = font();
    const int targetHeight = rect().height() - 8;
    const int targetWidth = rect().width() - 8;
    if (targetHeight <= 0 || targetWidth <= 0) return;

    qreal pointSize = qMax(current.pointSizeF(), 6.0);
    for (int guard = 0; guard < 64; ++guard) {
        current.setPointSizeF(pointSize);
        const QRect bounds = QFontMetrics(current).boundingRect(text());
        if (bounds.height() <= targetHeight && bounds.width() <= targetWidth) {
            if (pointSize >= 48.0) break;
            pointSize += 0.5;
        } else {
            pointSize -= 0.5;
            break;
        }
    }
    current.setPointSizeF(qMax(pointSize, 6.0));
    setFont(current);
}

} // namespace qkw
