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
    // Loading
    void loadsValidJson();
    void invalidJsonReportsError();
    void invalidJsonEmitsLoadFailedNotSourceChanged();
    void reloadingJsonResetsToFirstPage();
    void setSourceTriggersLoad();
    void setSourceWithBadPathEmitsLoadFailedNotSourceChanged();

    // Signal emissions
    void emitsCharacterEnteredForCharKey();
    void emitsSpaceForSpaceKey();
    void emitsBackspaceRequested();
    void emitsEnterRequested();

    // Page switching
    void shiftKeySwitchesPage();
    void switchKeySwitchesPage();
    void setCurrentPageIndexDirectly();
    void setCurrentPageIndexOutOfRangeIsIgnored();
    void setPageByIdWorks();
    void setPageByIdUnknownIsIgnored();

    // rows() shape
    void rowsReflectsCurrentPage();
    void rowsForPageReturnsCorrectPage();
    void rowsContainsExpectedFields();

    // Boundary / safety
    void activateKeyAtOutOfRangeIsIgnored();
    void activateKeyAtOnInvalidControllerIsIgnored();
};

// ---------------------------------------------------------------------------
// Loading
// ---------------------------------------------------------------------------

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

void TestKeyboardController::invalidJsonReportsError()
{
    KeyboardController controller;
    QVERIFY(!controller.loadJson(QByteArrayLiteral("not json")));
    QVERIFY(!controller.isValid());
    QVERIFY(!controller.errorString().isEmpty());
}

void TestKeyboardController::invalidJsonEmitsLoadFailedNotSourceChanged()
{
    // Issue #47: a failed load must be observable via a dedicated signal
    // rather than leaving callers to guess from sourceChanged() never
    // firing — sourceChanged() only fires on a *successful* load. Load a
    // valid layout first so the failure below has real prior state to
    // (not) clobber, matching the documented "leaves prior state in place"
    // contract.
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy failedSpy(&controller, &KeyboardController::loadFailed);
    QSignalSpy sourceSpy(&controller, &KeyboardController::sourceChanged);
    QSignalSpy layoutSpy(&controller, &KeyboardController::layoutChanged);

    QVERIFY(!controller.loadJson(QByteArrayLiteral("not json")));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(0).toString(), controller.errorString());
    QVERIFY(!controller.errorString().isEmpty());
    QCOMPARE(layoutSpy.count(), 1);
    QCOMPARE(sourceSpy.count(), 0);

    // The previously-loaded layout must still be intact.
    QVERIFY(controller.isValid());
    QCOMPARE(controller.pageCount(), 2);
    QCOMPARE(controller.currentPageId(), QStringLiteral("lower"));
}

void TestKeyboardController::reloadingJsonResetsToFirstPage()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));
    controller.setCurrentPageIndex(1); // go to "upper"
    QCOMPARE(controller.currentPageIndex(), 1);

    // Reload resets page index to 0.
    QVERIFY(controller.loadJson(sampleLayoutJson()));
    QCOMPARE(controller.currentPageIndex(), 0);
    QCOMPARE(controller.currentPageId(), QStringLiteral("lower"));
}

void TestKeyboardController::setSourceTriggersLoad()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    KeyboardController controller;
    // source property starts empty and setting it to the same value is a no-op.
    QVERIFY(controller.source().isEmpty());

    QSignalSpy layoutSpy(&controller, &KeyboardController::layoutChanged);
    // Setting a valid path should load it and emit layoutChanged.
    controller.setSource(QStringLiteral(":/layouts/en.json"));
    QVERIFY(controller.isValid());
    QCOMPARE(layoutSpy.count(), 1);

    // Setting the same path again must be a no-op (no redundant signals).
    controller.setSource(QStringLiteral(":/layouts/en.json"));
    QCOMPARE(layoutSpy.count(), 1);
}

void TestKeyboardController::setSourceWithBadPathEmitsLoadFailedNotSourceChanged()
{
    // Issue #47: setSource() is the Q_PROPERTY WRITE function, so it can
    // only be called from QML/setProperty() as a fire-and-forget void — a
    // failed write must still be observable somehow, via loadFailed(). Set
    // a valid source first so the failure below has real prior state
    // (source()/valid()) to (not) clobber.
    Q_INIT_RESOURCE(qkeyboardwidget);

    KeyboardController controller;
    controller.setSource(QStringLiteral(":/layouts/en.json"));
    QVERIFY(controller.isValid());

    QSignalSpy failedSpy(&controller, &KeyboardController::loadFailed);
    QSignalSpy sourceSpy(&controller, &KeyboardController::sourceChanged);
    QSignalSpy layoutSpy(&controller, &KeyboardController::layoutChanged);

    controller.setSource(QStringLiteral("/nonexistent/path.json"));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(0).toString(), controller.errorString());
    QVERIFY(!controller.errorString().isEmpty());
    QCOMPARE(layoutSpy.count(), 1);
    QCOMPARE(sourceSpy.count(), 0);

    // The previously-loaded layout and source must still be intact.
    QVERIFY(controller.isValid());
    QCOMPARE(controller.source(), QStringLiteral(":/layouts/en.json"));
}

// ---------------------------------------------------------------------------
// Signal emissions
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Page switching
// ---------------------------------------------------------------------------

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

void TestKeyboardController::switchKeySwitchesPage()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "lower", "rows": [
                [ { "type": "switch", "target": "numeric", "labelId": "numbers" } ]
            ] },
            { "id": "numeric", "rows": [ [ { "type": "char", "text": "1" } ] ] }
        ]
    })";

    KeyboardController controller;
    QVERIFY(controller.loadJson(json));

    QSignalSpy spy(&controller, &KeyboardController::currentPageChanged);
    controller.activateKeyAt(0, 0); // the switch key, target "numeric"
    QCOMPARE(spy.count(), 1);
    QCOMPARE(controller.currentPageIndex(), 1);
    QCOMPARE(controller.currentPageId(), QStringLiteral("numeric"));
}

void TestKeyboardController::setCurrentPageIndexDirectly()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::currentPageChanged);
    controller.setCurrentPageIndex(1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(controller.currentPageIndex(), 1);

    // Setting the same index again must not emit the signal again.
    controller.setCurrentPageIndex(1);
    QCOMPARE(spy.count(), 1);
}

void TestKeyboardController::setCurrentPageIndexOutOfRangeIsIgnored()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::currentPageChanged);
    controller.setCurrentPageIndex(-1);
    controller.setCurrentPageIndex(99);
    QCOMPARE(spy.count(), 0);
    QCOMPARE(controller.currentPageIndex(), 0);
}

void TestKeyboardController::setPageByIdWorks()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    controller.setPageById(QStringLiteral("upper"));
    QCOMPARE(controller.currentPageIndex(), 1);
    QCOMPARE(controller.currentPageId(), QStringLiteral("upper"));
}

void TestKeyboardController::setPageByIdUnknownIsIgnored()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy spy(&controller, &KeyboardController::currentPageChanged);
    controller.setPageById(QStringLiteral("nonexistent"));
    QCOMPARE(spy.count(), 0);
    QCOMPARE(controller.currentPageIndex(), 0);
}

// ---------------------------------------------------------------------------
// rows() / rowsForPage() shape
// ---------------------------------------------------------------------------

void TestKeyboardController::rowsReflectsCurrentPage()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    // Page 0 ("lower") has 1 row with 5 keys.
    QCOMPARE(controller.rows().size(), 1);
    controller.setCurrentPageIndex(1); // "upper" has 1 row with 1 key.
    QCOMPARE(controller.rows().size(), 1);
    const QVariantList upperRow = controller.rows().first().toList();
    QCOMPARE(upperRow.size(), 1);
}

void TestKeyboardController::rowsForPageReturnsCorrectPage()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    const QVariantList lowerRows = controller.rowsForPage(0);
    QCOMPARE(lowerRows.size(), 1);
    QCOMPARE(lowerRows.first().toList().size(), 5); // 5 keys on "lower"

    const QVariantList upperRows = controller.rowsForPage(1);
    QCOMPARE(upperRows.size(), 1);
    QCOMPARE(upperRows.first().toList().size(), 1); // 1 key on "upper"
}

void TestKeyboardController::rowsContainsExpectedFields()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    // Each key map must expose: row, column, action, text, icon, target, span.
    const QVariantMap keyMap = controller.rows().first().toList().first().toMap();
    QVERIFY(keyMap.contains(QStringLiteral("row")));
    QVERIFY(keyMap.contains(QStringLiteral("column")));
    QVERIFY(keyMap.contains(QStringLiteral("action")));
    QVERIFY(keyMap.contains(QStringLiteral("text")));
    QVERIFY(keyMap.contains(QStringLiteral("icon")));
    QVERIFY(keyMap.contains(QStringLiteral("target")));
    QVERIFY(keyMap.contains(QStringLiteral("span")));

    QCOMPARE(keyMap.value(QStringLiteral("row")).toInt(), 0);
    QCOMPARE(keyMap.value(QStringLiteral("column")).toInt(), 0);
    QCOMPARE(keyMap.value(QStringLiteral("text")).toString(), QStringLiteral("a"));
    QCOMPARE(keyMap.value(QStringLiteral("span")).toInt(), 1);
}

// ---------------------------------------------------------------------------
// Boundary / safety
// ---------------------------------------------------------------------------

void TestKeyboardController::activateKeyAtOutOfRangeIsIgnored()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(sampleLayoutJson()));

    QSignalSpy charSpy(&controller, &KeyboardController::characterEntered);
    QSignalSpy bsSpy(&controller, &KeyboardController::backspaceRequested);

    controller.activateKeyAt(-1, 0);
    controller.activateKeyAt(0, -1);
    controller.activateKeyAt(99, 0);
    controller.activateKeyAt(0, 99);

    QCOMPARE(charSpy.count(), 0);
    QCOMPARE(bsSpy.count(), 0);
}

void TestKeyboardController::activateKeyAtOnInvalidControllerIsIgnored()
{
    KeyboardController controller; // no layout loaded

    QSignalSpy charSpy(&controller, &KeyboardController::characterEntered);
    controller.activateKeyAt(0, 0);
    QCOMPARE(charSpy.count(), 0);
}

QTEST_GUILESS_MAIN(TestKeyboardController)
#include "tst_keyboardcontroller.moc"
