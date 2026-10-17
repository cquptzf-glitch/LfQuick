import QtQuick
import QtQuick.Controls as Controls

Controls.RangeSlider {
    id: control

    implicitWidth: 220
    implicitHeight: Tokens.controlHeight

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 4
        radius: height / 2
        color: Tokens.surfaceTertiary

        Rectangle {
            x: control.first.visualPosition * parent.width
            width: Math.max(0, (control.second.visualPosition - control.first.visualPosition) * parent.width)
            height: parent.height
            radius: parent.radius
            color: control.enabled ? Tokens.accent : Tokens.textDisabled
        }
    }

    first.handle: Rectangle {
        x: control.leftPadding + control.first.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: Tokens.indicatorSize
        implicitHeight: Tokens.indicatorSize
        radius: width / 2
        color: control.enabled ? (control.first.pressed ? Tokens.accentPressed : Tokens.accent)
                               : Tokens.textDisabled
        border.width: control.first.hovered || control.activeFocus ? 3 : 2
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.surface
    }

    second.handle: Rectangle {
        x: control.leftPadding + control.second.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: Tokens.indicatorSize
        implicitHeight: Tokens.indicatorSize
        radius: width / 2
        color: control.enabled ? (control.second.pressed ? Tokens.accentPressed : Tokens.accent)
                               : Tokens.textDisabled
        border.width: control.second.hovered || control.activeFocus ? 3 : 2
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.surface
    }
}
