import QtQuick 2.0
import QtQuick.Controls 2.0
import QKeyboard 1.0

ApplicationWindow {
    id: window
    width: 560
    height: 420
    visible: true
    title: qsTr("QKeyboard QML Example")

    KeyboardController {
        id: controller
        // Already shows English by default; the combo box below switches it
        // via setLocale() rather than a "qrc:/layouts/..." source string.

        onCharacterEntered: (text) => inputField.insert(inputField.cursorPosition, text)
        onBackspaceRequested: {
            const from = Math.max(0, inputField.cursorPosition - 1)
            inputField.remove(from, inputField.cursorPosition)
        }
        onEnterRequested: inputField.text = ""
    }

    KeyboardTheme {
        id: theme
        keyColor: "#2b2b2e"
        accentKeyColor: "#ff9500"
        cornerRadius: 10
    }

    ComboBox {
        id: localeBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        textRole: "text"
        model: [
            { text: "English", locale: KeyboardController.English },
            { text: "한국어", locale: KeyboardController.Korean }
        ]
        onActivated: (index) => { controller.setLocale(model[index].locale) }
    }

    TextField {
        id: inputField
        anchors.top: localeBox.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        placeholderText: qsTr("Type using the keyboard below")
    }

    KeyboardPanel {
        anchors.top: inputField.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        controller: controller
        theme: theme
    }
}
