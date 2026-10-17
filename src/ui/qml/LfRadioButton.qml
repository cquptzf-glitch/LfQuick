import QtQuick
import QtQuick.Controls as Controls

Controls.RadioButton {
    id: control

    implicitHeight: Math.max(Tokens.controlHeight, indicator.implicitHeight)
    spacing: Tokens.controlSpacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: Tokens.indicatorSize
        implicitHeight: Tokens.indicatorSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: width / 2
        color: control.enabled ? Tokens.controlFill : Tokens.controlFillDisabled
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing
                    : control.checked ? Tokens.accent
                    : Tokens.borderStrong

        Rectangle {
            anchors.centerIn: parent
            width: Math.max(6, parent.width * 0.46)
            height: width
            radius: width / 2
            color: control.enabled ? Tokens.accent : Tokens.textDisabled
            visible: control.checked
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
