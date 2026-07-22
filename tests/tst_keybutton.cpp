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
};

void TestKeyButton::shrinksToFitInASingleResize()
{
    KeyButton button(QStringLiteral("Backspace"));
    QFont big = button.font();
    big.setPointSizeF(40.0);
    button.setFont(big);
    button.show();
    QVERIFY(QTest::qWaitForWindowExposed(&button));
    button.resize(60, 40);
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
    QVERIFY(QTest::qWaitForWindowExposed(&button));
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

QTEST_MAIN(TestKeyButton)
#include "tst_keybutton.moc"
