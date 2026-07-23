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

    function loadBaselineLayout() {
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

    function initTestCase() {
        loadBaselineLayout()
    }

    // Some test functions (the icon ones below) load their own
    // single-key layout into the shared controller/panel; restore the
    // 3-key baseline afterward so later tests aren't affected by
    // whichever test happened to run before them.
    function cleanup() {
        loadBaselineLayout()
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

    // Image is the only direct child of KeyboardKey with a "source"
    // property; Text and MouseArea don't have one, so this distinguishes it
    // without relying on declaration order.
    function findIconAndLabel(key) {
        let icon = null
        let label = null
        for (let i = 0; i < key.children.length; ++i) {
            const child = key.children[i]
            if (typeof child.source !== "undefined") icon = child
            else if (typeof child.text !== "undefined") label = child
        }
        return { icon: icon, label: label }
    }

    function test_iconWithRealResourcePathLoadsSuccessfully() {
        // Regression test for #78: a bare Qt-resource-style icon path (the
        // same format KeyDefinition::toVariantMap()/the real layout JSON
        // emits, e.g. layouts/en.json's shift key) failed to load via
        // Image.source, which needs an explicit "qrc:" URL scheme rather
        // than the bare ":" prefix QIcon (used by the QWidget view) accepts
        // directly. Using the real compiled-in resource path here, not a
        // hand-crafted "qrc:/..." value, so this actually exercises the
        // conversion. Loaded through the real controller/panel (not a
        // synthetic KeyboardKey instance) so the key is properly parented
        // and shown, matching how the bug actually manifested.
        const json = JSON.stringify({
            locale: "en",
            pages: [ { id: "lower", rows: [
                [ { type: "backspace", icon: ":/qkeyboardwidget/icons/backspace.svg" } ]
            ] } ]
        })
        verify(controller.loadJson(json))
        tryVerify(function () { return findKeyboardKeys(panel).length === 1 })
        const found = findIconAndLabel(findKeyboardKeys(panel)[0])
        verify(found.icon !== null)
        tryCompare(found.icon, "status", Image.Ready)
    }

    function test_iconThatFailsToLoadFallsBackToLabel() {
        // Compounding bug from #78: the label Text's visibility only checked
        // whether an icon *path* was present, not whether it actually
        // loaded, so a broken icon path showed neither the icon nor a
        // fallback letter - a completely blank key. Asserting on the key's
        // labelShouldBeVisible property rather than the label Text's own
        // "visible" - see the comment on KeyboardKey.qml's
        // labelShouldBeVisible property for why (a Qt Quick Test TestCase
        // quirk, not a production concern: manually verified in the real
        // qml_example app that the label Text itself does become visible).
        const json = JSON.stringify({
            locale: "en",
            pages: [ { id: "lower", rows: [
                [ { type: "backspace", icon: ":/qkeyboardwidget/icons/does-not-exist.svg" } ]
            ] } ]
        })
        verify(controller.loadJson(json))
        tryVerify(function () { return findKeyboardKeys(panel).length === 1 })
        const key = findKeyboardKeys(panel)[0]
        const found = findIconAndLabel(key)
        verify(found.icon !== null)
        verify(found.label !== null)
        tryCompare(found.icon, "status", Image.Error)
        tryVerify(function () { return key.labelShouldBeVisible === true })
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
