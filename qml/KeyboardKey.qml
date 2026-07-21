import QtQuick
import QKeyboardWidget

// A single key rendered from the {row, column, action, text, icon, span}
// map produced by KeyboardController.rows. Purely presentational: it emits
// activated() and leaves what that means to whoever owns the controller.
Rectangle {
    id: root

    property var keyData
    property KeyboardTheme theme

    signal activated()

    radius: theme ? theme.cornerRadius : 6
    color: mouseArea.pressed
           ? (theme ? theme.keyPressedColor : "#636366")
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
        visible: !(keyData && keyData.icon)
        anchors.centerIn: parent
        text: keyData ? keyData.text : ""
        color: theme ? theme.textColor : "white"
        font: theme ? theme.font : parent.font
        elide: Text.ElideNone
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.activated()
    }
}
