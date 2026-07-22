#include <QtTest>

#include "qkeyboardwidget/key_button.h"
#include "qkeyboardwidget/keyboard_controller.h"
#include "qkeyboardwidget/keyboard_theme.h"
#include "qkeyboardwidget/keyboard_widget.h"

using namespace qkw;

namespace {

QByteArray sampleLayoutJson()
{
    return R"({
        "locale": "en",
        "pages": [
            {
                "id": "lower",
                "rows": [
                    [
                        { "type": "char", "text": "a" },
                        { "type": "shift", "target": "upper" },
                        { "type": "switch", "target": "numeric", "labelId": "numbers" }
                    ]
                ]
            },
            { "id": "upper", "rows": [ [ { "type": "char", "text": "A" } ] ] },
            { "id": "numeric", "rows": [ [ { "type": "char", "text": "1" } ] ] }
        ]
    })";
}

} // namespace

class TestKeyboardWidget : public QObject
{
    Q_OBJECT
private slots:
    void buildsOneButtonPerKeyAcrossAllPages();
    void accentsShiftAndSwitchKeysOnly();
    void rebuildDoesNotCrashOnReload();
    void invalidLayoutKeepsPreviousButtons();
    void firstLoadInvalidLeavesNoButtons();
    void themeChangeRerendersExistingButtons();
    void showCurrentPageIsANoOpWithoutAValidLayout();
};

void TestKeyboardWidget::buildsOneButtonPerKeyAcrossAllPages()
{
    KeyboardWidget widget;
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));

    // 3 keys on "lower" + 1 on "upper" + 1 on "numeric" = 5 buttons total,
    // since rebuildPages() builds every page up front.
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 5);
}

void TestKeyboardWidget::accentsShiftAndSwitchKeysOnly()
{
    KeyboardWidget widget;
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));

    KeyButton *charButton = widget.findChild<KeyButton *>(QStringLiteral("a"));
    KeyButton *shiftButton = widget.findChild<KeyButton *>(QStringLiteral("Shift"));
    KeyButton *switchButton = widget.findChild<KeyButton *>(QStringLiteral("123"));
    QVERIFY(charButton);
    QVERIFY(shiftButton);
    QVERIFY(switchButton);

    QVERIFY(!charButton->isAccented());
    QVERIFY(shiftButton->isAccented());
    QVERIFY(switchButton->isAccented());
}

void TestKeyboardWidget::rebuildDoesNotCrashOnReload()
{
    KeyboardWidget widget;
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 5);

    // Reloading triggers rebuildPages() again while the previously built
    // pages are still pending deleteLater(); regression test for the
    // QStackedWidget double-delete fixed in #25.
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 5);
}

void TestKeyboardWidget::invalidLayoutKeepsPreviousButtons()
{
    KeyboardWidget widget;
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 5);

    // KeyboardController::loadJson() keeps the previous valid layout in
    // place on a parse failure (only errorString() changes) rather than
    // blanking the keyboard, so rebuildPages() rebuilds the same 5 buttons
    // from the still-valid old layout instead of clearing them.
    QVERIFY(!widget.controller()->loadJson(QByteArrayLiteral("not json")));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 5);
    QVERIFY(!widget.controller()->errorString().isEmpty());
}

void TestKeyboardWidget::firstLoadInvalidLeavesNoButtons()
{
    // Unlike invalidLayoutKeepsPreviousButtons above, this is the widget's
    // very first load attempt: there's no previously-valid layout to fall
    // back on, so the controller is genuinely invalid and rebuildPages()
    // must take its early-return path without crashing.
    KeyboardWidget widget;
    QVERIFY(!widget.controller()->loadJson(QByteArrayLiteral("not json")));
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 0);
}

void TestKeyboardWidget::themeChangeRerendersExistingButtons()
{
    KeyboardWidget widget;
    QVERIFY(widget.controller()->loadJson(sampleLayoutJson()));

    KeyButton *charButton = widget.findChild<KeyButton *>(QStringLiteral("a"));
    QVERIFY(charButton);

    const QColor newColor(QStringLiteral("#abcdef"));
    QVERIFY(!charButton->styleSheet().contains(newColor.name(QColor::HexArgb), Qt::CaseInsensitive));

    widget.theme()->setKeyColor(newColor);
    QVERIFY(charButton->styleSheet().contains(newColor.name(QColor::HexArgb), Qt::CaseInsensitive));
}

void TestKeyboardWidget::showCurrentPageIsANoOpWithoutAValidLayout()
{
    // showCurrentPage()'s "!_controller->isValid()" guard is defensive:
    // KeyboardController::setCurrentPageIndex() requires a valid layout
    // before it will ever emit currentPageChanged(), so nothing in the
    // public API can reach this branch. Invoke the private slot directly
    // via the meta-object system to exercise it without crashing.
    KeyboardWidget widget;
    QVERIFY(!widget.controller()->isValid());
    QVERIFY(QMetaObject::invokeMethod(&widget, "showCurrentPage"));
    QCOMPARE(widget.findChildren<KeyButton *>().size(), 0);
}

QTEST_MAIN(TestKeyboardWidget)
#include "tst_keyboardwidget.moc"
