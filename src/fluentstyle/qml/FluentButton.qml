import QtQuick
import QtQuick.Controls as Controls

Controls.Button {
    id: root

    implicitHeight: FluentTokens.controlHeight
    leftPadding: FluentTokens.padding
    rightPadding: FluentTokens.padding
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    contentItem: Text {
        text: root.text
        font: root.font
        font.pixelSize: FluentTokens.fontSize
        color: !root.enabled ? FluentTokens.textSecondary
             : root.highlighted ? FluentTokens.onAccent
             : FluentTokens.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: FluentTokens.radius
        color: !root.enabled ? FluentTokens.disabled
             : root.highlighted ? FluentTokens.accent
             : root.down ? FluentTokens.pressed
             : root.hovered ? FluentTokens.hover
             : FluentTokens.fill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent : FluentTokens.border
    }
}
