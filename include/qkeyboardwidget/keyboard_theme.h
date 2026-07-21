#pragma once

#include <QObject>
#include <QColor>
#include <QFont>

namespace qkw {

// Visual appearance knobs shared by the QWidget and QML views. Changing any
// property re-renders the keyboard immediately (both views listen to the
// changed signals) so a host application can re-skin a running keyboard.
//
// Registered for QML with qmlRegisterType() (see qml_registration.h) rather
// than the QML_ELEMENT macro, since QML_ELEMENT requires Qt 5.15+/Qt6 and
// this library targets Qt5 through the latest Qt6.
class KeyboardTheme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY changed)
    Q_PROPERTY(QColor keyColor READ keyColor WRITE setKeyColor NOTIFY changed)
    Q_PROPERTY(QColor keyPressedColor READ keyPressedColor WRITE setKeyPressedColor NOTIFY changed)
    Q_PROPERTY(QColor accentKeyColor READ accentKeyColor WRITE setAccentKeyColor NOTIFY changed)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY changed)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY changed)
    Q_PROPERTY(qreal cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY changed)
    Q_PROPERTY(qreal keySpacing READ keySpacing WRITE setKeySpacing NOTIFY changed)

public:
    explicit KeyboardTheme(QObject *parent = nullptr);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    QColor keyColor() const;
    void setKeyColor(const QColor &color);

    QColor keyPressedColor() const;
    void setKeyPressedColor(const QColor &color);

    // Used for keys that represent an active/selected state, e.g. Shift.
    QColor accentKeyColor() const;
    void setAccentKeyColor(const QColor &color);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    QFont font() const;
    void setFont(const QFont &font);

    qreal cornerRadius() const;
    void setCornerRadius(qreal radius);

    qreal keySpacing() const;
    void setKeySpacing(qreal spacing);

signals:
    void changed();

private:
    QColor _backgroundColor;
    QColor _keyColor;
    QColor _keyPressedColor;
    QColor _accentKeyColor;
    QColor _textColor;
    QFont _font;
    qreal _cornerRadius = 6.0;
    qreal _keySpacing = 4.0;
};

} // namespace qkw
