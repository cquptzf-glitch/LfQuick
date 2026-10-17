import QtQuick
import QtQuick.Controls as Controls

Controls.Switch {
    id: root

    implicitHeight: FluentTokens.controlHeight
    spacing: FluentTokens.spacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 20
        x: root.leftPadding
        y: (root.height - height) / 2
        radius: height / 2
        color: root.checked ? FluentTokens.accent
             : root.hovered ? FluentTokens.hover
             : FluentTokens.fill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent : FluentTokens.border

        Rectangle {
            width: 14
            height: 14
            radius: width / 2
            y: 3
            x: root.checked ? parent.width - width - 3 : 3
            color: root.checked ? FluentTokens.onAccent : FluentTokens.textSecondary
            Behavior on x { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        font.pixelSize: FluentTokens.fontSize
        color: root.enabled ? FluentTokens.text : FluentTokens.textSecondary
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
        elide: Text.ElideRight
    }
}
