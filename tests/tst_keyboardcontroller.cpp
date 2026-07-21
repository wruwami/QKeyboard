#include <QtTest>
#include <QSignalSpy>

#include "qkeyboardwidget/keyboard_controller.h"

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
                        { "type": "shift", "target": "upper", "span": 2 },
                        { "type": "backspace" },
                        { "type": "enter" },
                        { "type": "space" }
                    ]
                ]
            },
            {
                "id": "upper",
                "rows": [
                    [ { "type": "char", "text": "A" } ]
                ]
            }
        ]
    })";
}

} // namespace

class TestKeyboardController : public QObject
{
    Q_OBJECT

private slots:
    void loadsValidJson();
    void emitsCharacterEnteredForCharKey();
    void emitsSpaceForSpaceKey();
    void emitsBackspaceRequested();
    void emitsEnterRequested();
    void shiftKeySwitchesPage();
    void invalidJsonReportsError();
};

void TestKeyboardController::loadsValidJson()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));
    QVERIFY(controller.isValid());
    QCOMPARE(controller.locale(), QStringLiteral("en"));
    QCOMPARE(controller.currentPageIndex(), 0);
    QCOMPARE(controller.currentPageId(), QStringLiteral("lower"));
    QCOMPARE(controller.pageCount(), 2);
}

void TestKeyboardController::emitsCharacterEnteredForCharKey()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::characterEntered);
    controller.activateKeyAt(0, 0); // the "a" key
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("a"));
}

void TestKeyboardController::emitsSpaceForSpaceKey()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::characterEntered);
    controller.activateKeyAt(0, 4); // the space key
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral(" "));
}

void TestKeyboardController::emitsBackspaceRequested()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::backspaceRequested);
    controller.activateKeyAt(0, 2);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardController::emitsEnterRequested()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::enterRequested);
    controller.activateKeyAt(0, 3);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardController::shiftKeySwitchesPage()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::currentPageChanged);
    controller.activateKeyAt(0, 1); // the shift key, target "upper"
    QCOMPARE(spy.count(), 1);
    QCOMPARE(controller.currentPageIndex(), 1);
    QCOMPARE(controller.currentPageId(), QStringLiteral("upper"));
}

void TestKeyboardController::invalidJsonReportsError()
{
    KeyboardController controller;
    QVERIFY(!controller.loadJson(QByteArrayLiteral("not json")));
    QVERIFY(!controller.isValid());
    QVERIFY(!controller.errorString().isEmpty());
}

QTEST_GUILESS_MAIN(TestKeyboardController)
#include "tst_keyboardcontroller.moc"
