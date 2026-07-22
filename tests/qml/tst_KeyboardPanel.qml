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

    function initTestCase() {
        const json = JSON.stringify({
            locale: "en",
            pages: [ { id: "lower", rows: [
                [ { type: "char", text: "a" }, { type: "char", text: "b" } ],
                [ { type: "char", text: "c" } ]
            ] } ]
        })
        verify(controller.loadJson(json))
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
        characterSpy.clear()
        const keys = findKeyboardKeys(panel)
        mouseClick(keys[1])
        compare(characterSpy.count, 1)
        compare(characterSpy.signalArguments[0][0], "b")
    }
}
