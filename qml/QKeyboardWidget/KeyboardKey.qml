import QtQuick 2.0
import QKeyboardWidget 1.0

// A single key rendered from the {row, column, action, text, icon, span}
// map produced by KeyboardController.rows. Purely presentational: it emits
// activated() and leaves what that means to whoever owns the controller.
Rectangle {
    id: root

    property var keyData
    property KeyboardTheme theme

    signal activated()

    // Shift (4) and Switch (5) keys use the accent colour so the user can tell
    // at a glance that pressing them changes the keyboard page rather than
    // typing a character. action is serialised as an int by toVariantMap()
    // matching the KeyAction enum: Character=0 Backspace=1 Enter=2 Space=3
    // Shift=4 Switch=5.
    readonly property bool isAccent: keyData && (keyData.action === 4 || keyData.action === 5)

    radius: theme ? theme.cornerRadius : 6
    color: mouseArea.pressed
           ? (theme ? theme.keyPressedColor : "#636366")
           : isAccent
             ? (theme ? theme.accentKeyColor : "#0a84ff")
             : (theme ? theme.keyColor : "#3a3a3c")

    Behavior on color { ColorAnimation { duration: 80 } }

    Image {
        visible: keyData && keyData.icon
        anchors.centerIn: parent
        source: keyData ? keyData.icon : ""
        width: Math.min(parent.width, parent.height) * 0.4
        height: width
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: keyLabel
        // Show label text only when there is no icon. When both are present
        // the icon takes precedence (matches KeyButton behaviour in QWidget).
        visible: !(keyData && keyData.icon)
        anchors.centerIn: parent
        text: keyData ? keyData.text : ""
        color: theme ? theme.textColor : "white"
        // A plain declarative binding works here because the fallback value
        // (Qt.application.font) doesn't reference keyLabel.font itself -
        // the earlier 'theme ? theme.font : font' version self-referenced
        // the property being assigned, which is what actually caused the
        // binding-loop warning (not the presence of theme.font). Reading
        // theme.font here also means this re-evaluates automatically on
        // KeyboardTheme::changed(), with no Connections/imperative handler
        // needed, and no Qt6 "implicitly defined onFoo" deprecation.
        font: theme ? theme.font : Qt.application.font
        elide: Text.ElideNone
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.activated()
    }
}
