import QtQuick
import QtQuick.Controls as Controls

Controls.RadioButton {
    id: root

    implicitHeight: FluentTokens.controlHeight
    spacing: FluentTokens.spacing
    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: root.leftPadding
        y: (root.height - height) / 2
        radius: width / 2
        color: root.hovered ? FluentTokens.hover : FluentTokens.fill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent
                    : root.checked ? FluentTokens.accent
                    : FluentTokens.border

        Rectangle {
            anchors.centerIn: parent
            width: 8
            height: 8
            radius: width / 2
            visible: root.checked
            color: FluentTokens.accent
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
