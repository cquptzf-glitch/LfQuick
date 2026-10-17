import QtQuick
import QtQuick.Controls as Controls

Controls.Button {
    id: root

    property bool destructive: false

    implicitHeight: BrandTokens.controlHeight
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true

    contentItem: Text {
        text: root.text
        font: root.font
        font.pixelSize: BrandTokens.fontSize
        color: !root.enabled ? BrandTokens.textSecondary
             : (root.highlighted || root.destructive) ? BrandTokens.onAccent
             : BrandTokens.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: BrandTokens.radius
        color: !root.enabled ? BrandTokens.surfaceRaised
             : root.destructive ? BrandTokens.destructive
             : root.highlighted ? BrandTokens.accent
             : root.down ? BrandTokens.pressed
             : root.hovered ? BrandTokens.hover
             : BrandTokens.control
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? BrandTokens.accent : BrandTokens.border
    }
}
