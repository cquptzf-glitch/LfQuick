import QtQuick
import QtQuick.Controls as Controls

Controls.CheckBox {
    id: root

    implicitHeight: FluentTokens.controlHeight
    spacing: FluentTokens.spacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: root.leftPadding
        y: (root.height - height) / 2
        radius: 3
        color: root.checked ? FluentTokens.accent
             : root.hovered ? FluentTokens.hover
             : FluentTokens.fill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent
                    : root.checked ? FluentTokens.accent
                    : FluentTokens.border

        Text {
            anchors.centerIn: parent
            visible: root.checked
            text: "✓"
            color: FluentTokens.onAccent
            font.pixelSize: 13
            font.bold: true
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
