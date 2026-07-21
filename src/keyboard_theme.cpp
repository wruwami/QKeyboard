#include "qkeyboardwidget/keyboard_theme.h"

namespace qkw {

KeyboardTheme::KeyboardTheme(QObject *parent) :
    QObject(parent),
    _backgroundColor(QStringLiteral("#1c1c1e")),
    _keyColor(QStringLiteral("#3a3a3c")),
    _keyPressedColor(QStringLiteral("#636366")),
    _accentKeyColor(QStringLiteral("#0a84ff")),
    _textColor(QStringLiteral("#ffffff"))
{
}

QColor KeyboardTheme::backgroundColor() const
{
    return _backgroundColor;
}

void KeyboardTheme::setBackgroundColor(const QColor &color)
{
    if (_backgroundColor == color) return;
    _backgroundColor = color;
    emit changed();
}

QColor KeyboardTheme::keyColor() const
{
    return _keyColor;
}

void KeyboardTheme::setKeyColor(const QColor &color)
{
    if (_keyColor == color) return;
    _keyColor = color;
    emit changed();
}

QColor KeyboardTheme::keyPressedColor() const
{
    return _keyPressedColor;
}

void KeyboardTheme::setKeyPressedColor(const QColor &color)
{
    if (_keyPressedColor == color) return;
    _keyPressedColor = color;
    emit changed();
}

QColor KeyboardTheme::accentKeyColor() const
{
    return _accentKeyColor;
}

void KeyboardTheme::setAccentKeyColor(const QColor &color)
{
    if (_accentKeyColor == color) return;
    _accentKeyColor = color;
    emit changed();
}

QColor KeyboardTheme::textColor() const
{
    return _textColor;
}

void KeyboardTheme::setTextColor(const QColor &color)
{
    if (_textColor == color) return;
    _textColor = color;
    emit changed();
}

QFont KeyboardTheme::font() const
{
    return _font;
}

void KeyboardTheme::setFont(const QFont &font)
{
    if (_font == font) return;
    _font = font;
    emit changed();
}

qreal KeyboardTheme::cornerRadius() const
{
    return _cornerRadius;
}

void KeyboardTheme::setCornerRadius(qreal radius)
{
    if (qFuzzyCompare(_cornerRadius, radius)) return;
    _cornerRadius = radius;
    emit changed();
}

qreal KeyboardTheme::keySpacing() const
{
    return _keySpacing;
}

void KeyboardTheme::setKeySpacing(qreal spacing)
{
    if (qFuzzyCompare(_keySpacing, spacing)) return;
    _keySpacing = spacing;
    emit changed();
}

} // namespace qkw
