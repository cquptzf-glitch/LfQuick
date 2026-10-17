import QtQuick
import QtQuick.Controls as Controls

Controls.Switch {
    id: root

    implicitHeight: BrandTokens.controlHeight
    spacing: BrandTokens.spacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: 46
        implicitHeight: 24
        x: root.leftPadding
        y: (root.height - height) / 2
        radius: height / 2
        color: !root.enabled ? BrandTokens.surfaceRaised
             : root.checked ? BrandTokens.accent
             : root.hovered ? BrandTokens.hover
             : BrandTokens.control
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? BrandTokens.accent : BrandTokens.border

        Rectangle {
            width: 18
            height: 18
            radius: 9
            y: 3
            x: root.checked ? parent.width - width - 3 : 3
            color: root.checked ? BrandTokens.onAccent : BrandTokens.textSecondary

            Behavior on x {
                NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
            }
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        font.pixelSize: BrandTokens.fontSize
        color: root.enabled ? BrandTokens.text : BrandTokens.textSecondary
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
        elide: Text.ElideRight
    }
}
