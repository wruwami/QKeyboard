import QtQuick 2.0
import QtTest 1.0
import QKeyboardWidget 1.0

TestCase {
    id: testCase
    name: "KeyboardPanel"
    when: windowShown
    width: 480
    height: 220

    KeyboardController { id: controller }
    KeyboardPanel { id: panel; anchors.fill: parent; controller: controller }

    SignalSpy {
        id: characterSpy
        target: controller
        signalName: "characterEntered"
    }

    function findKeyboardKeys(item) {
        let result = []
        for (let i = 0; i < item.children.length; ++i) {
            const child = item.children[i]
            if (typeof child.keyData !== "undefined") result.push(child)
            result = result.concat(findKeyboardKeys(child))
        }
        return result
    }

    function initTestCase() {
        const json = JSON.stringify({
            locale: "en",
            pages: [ { id: "lower", rows: [
                [ { type: "char", text: "a" }, { type: "char", text: "b" } ],
                [ { type: "char", text: "c" } ]
            ] } ]
        })
        verify(controller.loadJson(json))
        // Repeater delegates aren't created synchronously when `rows`
        // changes; poll until all 3 exist before any test function uses
        // them.
        tryVerify(function () {
            return findKeyboardKeys(panel).length === 3
        }, 3000, "keys did not finish being created in time")
    }

    function test_rendersEveryKeyOnEveryRow() {
        // Regression test for #24: an outer Repeater delegate shadowing the
        // injected modelData role made the inner Repeater's model resolve to
        // undefined and render zero keys.
        const keys = findKeyboardKeys(panel)
        compare(keys.length, 3)
        compare(keys[0].keyData.text, "a")
        compare(keys[1].keyData.text, "b")
        compare(keys[2].keyData.text, "c")
    }

    function test_clickForwardsRowAndColumnToController() {
        // Exercises the actual wiring this test cares about — the
        // per-delegate "onActivated: root.controller.activateKeyAt(
        // modelData.row, modelData.column)" line in KeyboardPanel.qml — by
        // emitting KeyboardKey's own activated() signal directly, the same
        // signal its MouseArea's onClicked emits. mouseClick()'s synthetic
        // input isn't reliably delivered to Repeater-created delegates in
        // this Qt Quick Test/offscreen-platform combination (confirmed via
        // an activated()-count probe: geometry and modelData.row/column
        // were correct, but zero clicks ever reached the MouseArea), and
        // that delivery mechanism is Qt's own MouseArea/QQuickWindow code,
        // not something this project owns or needs to regression-test.
        characterSpy.clear()
        const keys = findKeyboardKeys(panel)
        const k = keys[1]
        compare(k.keyData.row, 0)
        compare(k.keyData.column, 1)
        k.activated()
        compare(characterSpy.count, 1)
        compare(characterSpy.signalArguments[0][0], "b")
    }
}
