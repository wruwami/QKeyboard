import QtQuick 2.0
import QtQuick.Controls 2.0
import QKeyboardWidget 1.0

ApplicationWindow {
    id: window
    width: 560
    height: 420
    visible: true
    title: qsTr("QKeyboard QML Example")

    KeyboardController {
        id: controller
        source: "qrc:/layouts/en.json"
        onEnterRequested: {
            composer.commit()
            inputField.text = ""
        }
    }

    HangulComposer {
        id: composer
        controller: controller
        onCharacterEntered: (text) => inputField.insert(inputField.cursorPosition, text)
        onBackspaceRequested: {
            const from = Math.max(0, inputField.cursorPosition - 1)
            inputField.remove(from, inputField.cursorPosition)
        }
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
            { text: "English", source: "qrc:/layouts/en.json" },
            { text: "한국어", source: "qrc:/layouts/ko.json" }
        ]
        onActivated: (index) => { controller.source = model[index].source }
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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        height: 220
        controller: controller
        theme: theme
    }
}
