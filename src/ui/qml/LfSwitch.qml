import QtQuick
import QtQuick.Controls as Controls

Controls.Switch {
    id: control

    implicitHeight: Math.max(Tokens.controlHeight, indicator.implicitHeight)
    spacing: Tokens.controlSpacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: Tokens.indicatorSize * 2
        implicitHeight: Tokens.indicatorSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: height / 2
        color: !control.enabled ? Tokens.controlFillDisabled
             : control.checked ? Tokens.accent
             : control.hovered ? Tokens.controlFillHover
             : Tokens.controlFill
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing
                    : control.checked ? Tokens.accent
                    : Tokens.borderStrong

        Rectangle {
            id: thumb
            width: parent.height - 6
            height: width
            radius: width / 2
            y: 3
            x: control.checked ? parent.width - width - 3 : 3
            color: control.checked ? Tokens.onAccent
                                   : control.enabled ? Tokens.textSecondary : Tokens.textDisabled

            Behavior on x {
                NumberAnimation { duration: Tokens.animationFast; easing.type: Easing.OutCubic }
            }
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
