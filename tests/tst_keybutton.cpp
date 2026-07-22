#include <QtTest>

#include "qkeyboardwidget/key_button.h"
#include "qkeyboardwidget/keyboard_theme.h"

using namespace qkw;

class TestKeyButton : public QObject
{
    Q_OBJECT
private slots:
    void shrinksToFitInASingleResize();
    void growsToFillLargeButton();
    void applyThemeUsesAccentColorWhenAccented();
    void applyThemeUsesKeyColorWhenNotAccented();
    void isAccentedDefaultsToFalse();
    void emptyTextSkipsFontFitOnResize();
    void shrinkingBelowMarginSkipsFontFit();
};

void TestKeyButton::shrinksToFitInASingleResize()
{
    KeyButton button(QStringLiteral("Backspace"));
    QFont big = button.font();
    big.setPointSizeF(40.0);
    button.setFont(big);
    button.show();
    button.resize(120, 40);
    QCoreApplication::processEvents();

    const QFontMetrics metrics(button.font());
    const QRect bounds = metrics.boundingRect(button.text());
    QVERIFY2(bounds.width() <= button.width() - 8, "text should fit horizontally after a single resize");
    QVERIFY2(bounds.height() <= button.height() - 8, "text should fit vertically after a single resize");
}

void TestKeyButton::growsToFillLargeButton()
{
    KeyButton button(QStringLiteral("A"));
    const qreal initialPointSize = button.font().pointSizeF();
    button.show();
    button.resize(300, 300);
    QCoreApplication::processEvents();

    QVERIFY(button.font().pointSizeF() > initialPointSize);
}

void TestKeyButton::applyThemeUsesAccentColorWhenAccented()
{
    KeyboardTheme theme;
    theme.setKeyColor(QColor(QStringLiteral("#111111")));
    theme.setAccentKeyColor(QColor(QStringLiteral("#ff9500")));

    KeyButton button(QStringLiteral("Shift"));
    button.setAccented(true);
    button.applyTheme(theme);

    QVERIFY(button.styleSheet().contains(theme.accentKeyColor().name(QColor::HexArgb), Qt::CaseInsensitive));
    QVERIFY(!button.styleSheet().contains(theme.keyColor().name(QColor::HexArgb), Qt::CaseInsensitive));
}

void TestKeyButton::applyThemeUsesKeyColorWhenNotAccented()
{
    KeyboardTheme theme;
    theme.setKeyColor(QColor(QStringLiteral("#111111")));
    theme.setAccentKeyColor(QColor(QStringLiteral("#ff9500")));

    KeyButton button(QStringLiteral("a"));
    button.applyTheme(theme);

    QVERIFY(button.styleSheet().contains(theme.keyColor().name(QColor::HexArgb), Qt::CaseInsensitive));
}

void TestKeyButton::isAccentedDefaultsToFalse()
{
    KeyButton button(QStringLiteral("a"));
    QVERIFY(!button.isAccented());
    button.setAccented(true);
    QVERIFY(button.isAccented());
    button.setAccented(false);
    QVERIFY(!button.isAccented());
}

void TestKeyButton::emptyTextSkipsFontFitOnResize()
{
    // fitFontToButton() returns immediately for an empty label; resizing
    // must not crash and must leave the font untouched.
    KeyButton button(QStringLiteral(""));
    const QFont initial = button.font();
    button.show();
    button.resize(120, 40);
    QCoreApplication::processEvents();
    QCOMPARE(button.font(), initial);
}

void TestKeyButton::shrinkingBelowMarginSkipsFontFit()
{
    // fitFontToButton() returns immediately once the button is smaller than
    // its fixed 8px margin in either dimension; must not crash.
    KeyButton button(QStringLiteral("a"));
    const QFont initial = button.font();
    button.setMinimumSize(0, 0); // KeyButton's own ctor sets a 24x24 floor
    button.show();
    button.resize(4, 4);
    QCoreApplication::processEvents();
    QCOMPARE(button.font(), initial);
}

QTEST_MAIN(TestKeyButton)
#include "tst_keybutton.moc"
