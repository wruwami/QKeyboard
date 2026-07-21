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
        // font is applied imperatively to avoid a binding-loop warning.
        // A declarative 'font: theme ? theme.font : font' self-references the
        // property and triggers "Binding loop detected" in QML. Using an
        // onThemeChanged handler is safe on all Qt5/Qt6 versions.
        elide: Text.ElideNone
    }

    // Apply theme.font whenever the theme object itself changes. The 'changed'
    // signal on KeyboardTheme (which fires for any property mutation) is wired
    // via Connections below, so font updates propagate at runtime too.
    onThemeChanged: {
        if (theme) keyLabel.font = theme.font
    }

    Connections {
        target: theme
        // Update font whenever any theme property changes (the theme emits a
        // single 'changed' signal for all properties).
        function onChanged() { if (theme) keyLabel.font = theme.font }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.activated()
    }
}
