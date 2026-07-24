import QtQuick 2.0
import QtTest 1.0
import QKeyboard 1.0

TestCase {
    id: testCase
    name: "KeyboardKey"
    when: windowShown
    width: 100
    height: 100

    KeyboardTheme {
        id: theme
        keyColor: "#111111"
        accentKeyColor: "#ff9500"
    }

    Component {
        id: keyComponent
        KeyboardKey {}
    }

    // action ints per KeyDefinition::toVariantMap(): Character=0 Backspace=1
    // Enter=2 Space=3 Shift=4 Switch=5.
    function makeKey(action) {
        return keyComponent.createObject(testCase, {
            theme: theme,
            keyData: { action: action, text: "x", icon: "" }
        })
    }

    function test_accentColorAppliedToShiftAndSwitchOnly() {
        let key = makeKey(0)
        compare(key.color, theme.keyColor)
        key.destroy()

        key = makeKey(4)
        compare(key.color, theme.accentKeyColor)
        key.destroy()

        key = makeKey(5)
        compare(key.color, theme.accentKeyColor)
        key.destroy()

        key = makeKey(1)
        compare(key.color, theme.keyColor)
        key.destroy()
    }
}
