import QtQuick
import QtQuick.Controls as Controls

Controls.Button {
    id: control

    property bool destructive: false

    implicitWidth: Math.max(80, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? Tokens.textDisabled
             : (control.highlighted || control.destructive) ? Tokens.onAccent
             : Tokens.textPrimary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: {
            if (!control.enabled)
                return Tokens.controlFillDisabled
            if (control.destructive)
                return control.down ? Qt.darker(Tokens.error, 1.12)
                     : control.hovered ? Qt.lighter(Tokens.error, 1.08)
                     : Tokens.error
            if (control.highlighted)
                return control.down ? Tokens.accentPressed
                     : control.hovered ? Tokens.accentHover
                     : Tokens.accent
            return control.down ? Tokens.controlFillPressed
                 : control.hovered ? Tokens.controlFillHover
                 : Tokens.controlFill
        }
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.border

        Behavior on color {
            ColorAnimation { duration: Tokens.animationFast }
        }
    }
}
