#include <QSignalSpy>
#include <QtTest>

#include "qkeyboardwidget/keyboard_theme.h"

using namespace qkw;

class TestKeyboardTheme : public QObject
{
    Q_OBJECT
private slots:
    void defaultsAreSet();

    void setBackgroundColorEmitsChanged();
    void setBackgroundColorToSameValueIsNoOp();

    void setKeyColorEmitsChanged();
    void setKeyColorToSameValueIsNoOp();

    void setKeyPressedColorEmitsChanged();
    void setKeyPressedColorToSameValueIsNoOp();

    void setAccentKeyColorEmitsChanged();
    void setAccentKeyColorToSameValueIsNoOp();

    void setTextColorEmitsChanged();
    void setTextColorToSameValueIsNoOp();

    void setFontEmitsChanged();
    void setFontToSameValueIsNoOp();

    void setCornerRadiusEmitsChanged();
    void setCornerRadiusToSameValueIsNoOp();

    void setKeySpacingEmitsChanged();
    void setKeySpacingToSameValueIsNoOp();
};

void TestKeyboardTheme::defaultsAreSet()
{
    KeyboardTheme theme;
    QVERIFY(theme.backgroundColor().isValid());
    QVERIFY(theme.keyColor().isValid());
    QVERIFY(theme.keyPressedColor().isValid());
    QVERIFY(theme.accentKeyColor().isValid());
    QVERIFY(theme.textColor().isValid());
    QCOMPARE(theme.cornerRadius(), 6.0);
    QCOMPARE(theme.keySpacing(), 4.0);
}

void TestKeyboardTheme::setBackgroundColorEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setBackgroundColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(theme.backgroundColor(), QColor(QStringLiteral("#112233")));
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setBackgroundColorToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QColor initial = theme.backgroundColor();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setBackgroundColor(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setKeyColorEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeyColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(theme.keyColor(), QColor(QStringLiteral("#112233")));
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setKeyColorToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QColor initial = theme.keyColor();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeyColor(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setKeyPressedColorEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeyPressedColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(theme.keyPressedColor(), QColor(QStringLiteral("#112233")));
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setKeyPressedColorToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QColor initial = theme.keyPressedColor();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeyPressedColor(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setAccentKeyColorEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setAccentKeyColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(theme.accentKeyColor(), QColor(QStringLiteral("#112233")));
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setAccentKeyColorToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QColor initial = theme.accentKeyColor();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setAccentKeyColor(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setTextColorEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setTextColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(theme.textColor(), QColor(QStringLiteral("#112233")));
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setTextColorToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QColor initial = theme.textColor();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setTextColor(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setFontEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    QFont font;
    font.setPointSizeF(20.0);
    theme.setFont(font);
    QCOMPARE(theme.font(), font);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setFontToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const QFont initial = theme.font();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setFont(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setCornerRadiusEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setCornerRadius(12.0);
    QCOMPARE(theme.cornerRadius(), 12.0);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setCornerRadiusToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const qreal initial = theme.cornerRadius();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setCornerRadius(initial);
    QCOMPARE(spy.count(), 0);
}

void TestKeyboardTheme::setKeySpacingEmitsChanged()
{
    KeyboardTheme theme;
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeySpacing(8.0);
    QCOMPARE(theme.keySpacing(), 8.0);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardTheme::setKeySpacingToSameValueIsNoOp()
{
    KeyboardTheme theme;
    const qreal initial = theme.keySpacing();
    QSignalSpy spy(&theme, &KeyboardTheme::changed);
    theme.setKeySpacing(initial);
    QCOMPARE(spy.count(), 0);
}

QTEST_GUILESS_MAIN(TestKeyboardTheme)
#include "tst_keyboardtheme.moc"
