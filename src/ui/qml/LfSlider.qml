import QtQuick
import QtQuick.Controls as Controls

Controls.Slider {
    id: control

    implicitWidth: 180
    implicitHeight: Tokens.controlHeight
    hoverEnabled: true

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 4
        radius: height / 2
        color: Tokens.surfaceTertiary

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: control.enabled ? Tokens.accent : Tokens.textDisabled
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: Tokens.indicatorSize
        implicitHeight: Tokens.indicatorSize
        radius: width / 2
        color: control.enabled ? (control.pressed ? Tokens.accentPressed : Tokens.accent)
                               : Tokens.textDisabled
        border.width: control.activeFocus ? 3 : 2
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.surface
    }
}
