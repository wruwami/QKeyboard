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
             ? (theme ? theme.accentKeyColor : "#ff9500")
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
        // Show label text only when there is no icon. When both are present
        // the icon takes precedence (matches KeyButton behaviour in QWidget).
        visible: !(keyData && keyData.icon)
        anchors.centerIn: parent
        text: keyData ? keyData.text : ""
        color: theme ? theme.textColor : "white"
        // Avoid binding to parent.font: Rectangle has no font property and
        // the binding would produce a QML warning. Fall back to the Text
        // element's own default font when no theme is set.
        font: theme ? theme.font : font
        elide: Text.ElideNone
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.activated()
    }
}
