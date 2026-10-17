import QtQuick
import QtQuick.Controls as Controls

Controls.CheckBox {
    id: control

    implicitHeight: Math.max(Tokens.controlHeight, indicator.implicitHeight)
    spacing: Tokens.controlSpacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: Tokens.indicatorSize
        implicitHeight: Tokens.indicatorSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: Tokens.radiusSmall
        color: !control.enabled ? Tokens.controlFillDisabled
             : control.checked ? Tokens.accent
             : control.down ? Tokens.controlFillPressed
             : control.hovered ? Tokens.controlFillHover
             : Tokens.controlFill
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing
                    : control.checked ? Tokens.accent
                    : Tokens.borderStrong

        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: control.checked
            color: Tokens.onAccent
            font.bold: true
            font.pixelSize: Math.max(10, parent.height - 6)
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        font.pixelSize: Tokens.fontSizeBody
        color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        elide: Text.ElideRight
    }
}
