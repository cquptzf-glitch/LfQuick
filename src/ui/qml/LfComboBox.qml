import QtQuick
import QtQuick.Controls as Controls

Controls.ComboBox {
    id: control

    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal + 18
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true
    font.pixelSize: Tokens.fontSizeBody

    contentItem: Text {
        leftPadding: 0
        rightPadding: 0
        text: control.displayText
        font: control.font
        color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: control.width - width - Tokens.controlPaddingHorizontal
        y: (control.height - height) / 2
        text: "⌄"
        color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
        font.pixelSize: Tokens.fontSizeBody + 2
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: !control.enabled ? Tokens.controlFillDisabled
             : control.down ? Tokens.controlFillPressed
             : control.hovered ? Tokens.controlFillHover
             : Tokens.controlFill
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.border
    }
}
