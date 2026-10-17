import QtQuick
import QtQuick.Controls as Controls

Controls.Slider {
    id: root

    implicitWidth: 180
    implicitHeight: FluentTokens.controlHeight

    background: Rectangle {
        x: root.leftPadding
        y: root.topPadding + root.availableHeight / 2 - height / 2
        width: root.availableWidth
        height: 4
        radius: 2
        color: FluentTokens.surfaceSecondary

        Rectangle {
            width: root.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: root.enabled ? FluentTokens.accent : FluentTokens.textSecondary
        }
    }

    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + root.availableHeight / 2 - height / 2
        implicitWidth: root.pressed ? 20 : 18
        implicitHeight: implicitWidth
        radius: width / 2
        color: root.enabled ? FluentTokens.accent : FluentTokens.textSecondary
        border.width: 3
        border.color: FluentTokens.surface
    }
}
