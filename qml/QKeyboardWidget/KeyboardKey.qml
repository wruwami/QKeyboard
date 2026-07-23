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

    readonly property bool hasIcon: !!(keyData && keyData.icon)
    // Whether keyLabel should show its fallback text: no icon at all, or an
    // icon that was given but failed to load (issue #78). A plain root-level
    // property (not keyLabel.visible itself) so it stays observable from a
    // regression test even when this delegate sits under an ancestor Qt
    // Quick considers not-visible - such as Qt Quick Test's TestCase, which
    // stops propagating further changes to a not-visible item's own
    // "visible" property as a rendering optimization. Real host
    // applications don't have that ancestor, so keyLabel.visible (bound to
    // this below) updates normally for them.
    readonly property bool labelShouldBeVisible: !hasIcon || keyIcon.status === Image.Error

    radius: theme ? theme.cornerRadius : 6
    // Instant color swap on press, matching KeyButton's stylesheet-based
    // press feedback in the QWidget view (no animation on either side - see
    // issue #44 point 4).
    color: mouseArea.pressed
           ? (theme ? theme.keyPressedColor : "#636366")
           : isAccent
             ? (theme ? theme.accentKeyColor : "#0a84ff")
             : (theme ? theme.keyColor : "#3a3a3c")

    // These are root's own property-change signals, handled directly here
    // (not via a Connections{} attached to another object), so this is
    // ordinary Qt5/Qt6-compatible syntax throughout - keyLabel.fitFont()
    // itself reads root.width/root.height, not keyLabel's own size, since
    // keyLabel's size is a function of the font size fitFont() is solving
    // for (using keyLabel's own width/height here would be circular).
    onWidthChanged: keyLabel.fitFont()
    onHeightChanged: keyLabel.fitFont()

    Image {
        id: keyIcon
        // keyData.icon is a bare Qt-resource path (":/...", the same string
        // KeyButton's QIcon(iconPath) uses directly in the QWidget view) -
        // Image.source is a QML url property and only recognizes it with an
        // explicit "qrc:" scheme prepended (issue #78).
        visible: status === Image.Ready
        anchors.centerIn: parent
        source: root.hasIcon ? "qrc" + keyData.icon : ""
        // Matches KeyButton::fitIconToButton()'s 40%-of-the-smaller-dimension
        // rule in the QWidget view (issue #44 point 2).
        width: Math.min(parent.width, parent.height) * 0.4
        height: width
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: keyLabel
        // Show label text whenever there is no icon to show - including
        // when an icon path was given but failed to load, so a broken icon
        // still falls back to something visible instead of a blank key
        // (issue #78).
        visible: root.labelShouldBeVisible
        anchors.centerIn: parent
        text: keyData ? keyData.text : ""
        color: theme ? theme.textColor : "white"
        // Family/weight/style come from the theme (grouped-property
        // sub-bindings, not a whole-object 'font: theme.font' assignment,
        // so fitFont() below can independently drive font.pixelSize without
        // the two bindings fighting each other). The fallback values don't
        // reference this Text's own font, so no self-referencing binding
        // loop - see the KeyboardTheme font-sync fix in #50 for why that
        // matters here.
        font.family: theme ? theme.font.family : Qt.application.font.family
        font.bold: theme ? theme.font.bold : false
        font.italic: theme ? theme.font.italic : false
        elide: Text.ElideNone

        // Mirrors KeyButton::fitFontToButton() in the QWidget view: grow/
        // shrink pixelSize so the label fills the key, instead of using
        // theme.font's size verbatim - QML has no built-in equivalent
        // (issue #44 point 3). Binary search assumes fit is monotonic in
        // pixelSize, same assumption the QWidget version makes.
        function fitFont() {
            if (!text) return
            var targetW = root.width - 8
            var targetH = root.height - 8
            if (targetW <= 0 || targetH <= 0) return

            var lo = 6
            var hi = 96
            var best = lo
            while (lo <= hi) {
                var mid = Math.floor((lo + hi) / 2)
                font.pixelSize = mid
                if (contentWidth <= targetW && contentHeight <= targetH) {
                    best = mid
                    lo = mid + 1
                } else {
                    hi = mid - 1
                }
            }
            font.pixelSize = best
        }

        onTextChanged: fitFont()
        Component.onCompleted: fitFont()
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.activated()
    }
}
