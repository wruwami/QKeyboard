import QtQuick 2.0
import QtQuick.Layouts 1.0
import QKeyboardWidget 1.0

// Qt Quick view for a KeyboardController. Mirrors what KeyboardWidget does
// for QWidget apps: render whatever `controller.rows` currently holds and
// forward taps back into `controller.activateKeyAt(row, column)`.
Item {
    id: root

    property KeyboardController controller
    property KeyboardTheme theme: KeyboardTheme {}

    implicitWidth: 480
    implicitHeight: 220

    Rectangle {
        anchors.fill: parent
        color: root.theme.backgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.keySpacing
        spacing: root.theme.keySpacing

        Repeater {
            model: root.controller ? root.controller.rows : []

            delegate: RowLayout {
                property var modelData

                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.theme.keySpacing

                Repeater {
                    model: parent.modelData

                    delegate: KeyboardKey {
                        property var modelData

                        keyData: modelData
                        theme: root.theme
                        Layout.preferredWidth: 40 * (modelData.span || 1) + (root.theme.keySpacing * ((modelData.span || 1) - 1))
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onActivated: root.controller.activateKeyAt(modelData.row, modelData.column)
                    }
                }
            }
        }
    }
}
