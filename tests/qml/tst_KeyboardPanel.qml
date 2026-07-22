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
        // changes, and RowLayout/ColumnLayout compute width/height via
        // bindings before their separate arrange/polish pass actually sets
        // x/y — so a single waitForRendering() call right after loadJson()
        // isn't guaranteed to observe the *post-layout* frame. Poll for the
        // second key actually being positioned to the right of the first
        // (nonzero x) so mouseClick() below hits its real on-screen bounds
        // instead of a stale, still-overlapping-at-(0,0) position.
        tryVerify(function () {
            const keys = findKeyboardKeys(panel)
            return keys.length === 3 && keys[1].width > 0 && keys[1].height > 0 && keys[1].x > 0
        }, 3000, "keys did not finish laying out in time")
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
        const k = keys[1]
        console.log("keys[1] geometry: x=" + k.x + " y=" + k.y + " width=" + k.width
                     + " height=" + k.height + " mapped="
                     + JSON.stringify(k.mapToItem(null, k.width / 2, k.height / 2)))
        mouseClick(k)
        compare(characterSpy.count, 1)
        compare(characterSpy.signalArguments[0][0], "b")
    }
}
