import QtQuick 2.0
import QtQuick.Layouts 1.0
import QKeyboard 1.0

// Qt Quick view for a KeyboardController. Mirrors what KeyboardWidget does
// for QWidget apps: render whatever `controller.rows` currently holds and
// forward taps back into `controller.activateKeyAt(row, column)`.
//
// Uses a single GridLayout with each key placed at its real
// (Layout.row, Layout.column, Layout.columnSpan) - the same (row, column,
// span) values KeyboardWidget's QGridLayout uses - instead of independently
// width-sized RowLayouts. Column widths are therefore reconciled across the
// whole page exactly like QGridLayout does, including its known limitation
// when rows disagree on total span-units (see issue #44): that's a
// deliberate "match QWidget's behavior, quirks included" choice, not an
// accidental one - the alternative (per-row independent widths) was tried
// first and diverged from the QWidget view in the opposite direction.
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

    GridLayout {
        anchors.fill: parent
        anchors.margins: root.theme.keySpacing
        columnSpacing: root.theme.keySpacing
        rowSpacing: root.theme.keySpacing

        Repeater {
            // controller.rows is a list of rows of keys; flatten it into one
            // list so every key can be placed directly on the shared grid via
            // its own (row, column, span) instead of nesting a nested
            // Repeater-per-row (which is what produced independently-sized
            // rows before).
            //
            // key.column (as produced by KeyDefinition::toVariantMap) is the
            // key's ARRAY INDEX within its row - that's the value
            // KeyboardController::activateKeyAt(row, column) expects, since it
            // looks the key up via rowVec.at(column). It is NOT a real grid
            // column position once any earlier key in the row spans more than
            // one column, so it can't be used for Layout.column directly.
            // Instead we track a running cumulative-span offset per row here
            // (gridColumn) and keep the original array index (key.column)
            // untouched for activateKeyAt.
            model: {
                var flat = []
                var rows = root.controller ? root.controller.rows : []
                for (var r = 0; r < rows.length; ++r) {
                    var keyRow = rows[r]
                    var gridColumn = 0
                    for (var c = 0; c < keyRow.length; ++c) {
                        var key = keyRow[c]
                        flat.push({ keyData: key, gridColumn: gridColumn })
                        gridColumn += key.span || 1
                    }
                }
                return flat
            }

            delegate: KeyboardKey {
                required property var modelData

                keyData: modelData.keyData
                theme: root.theme
                Layout.row: modelData.keyData.row
                Layout.column: modelData.gridColumn
                Layout.columnSpan: modelData.keyData.span || 1
                Layout.fillWidth: true
                Layout.fillHeight: true

                onActivated: root.controller.activateKeyAt(modelData.keyData.row, modelData.keyData.column)
            }
        }
    }
}
