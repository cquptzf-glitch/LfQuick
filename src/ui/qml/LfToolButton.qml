import QtQuick
import QtQuick.Controls as Controls

Controls.ToolButton {
    id: control

    property bool compact: false

    implicitHeight: Tokens.controlHeight
    implicitWidth: compact
        ? implicitHeight
        : Math.max(implicitHeight, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: compact ? Units.sm : Tokens.controlPaddingHorizontal
    rightPadding: leftPadding
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: Units.sm

            Image {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.icon.source.toString().length > 0
                width: visible ? Units.iconSize : 0
                height: visible ? Units.iconSize : 0
                source: control.icon.source
                sourceSize.width: Units.iconSize
                sourceSize.height: Units.iconSize
                fillMode: Image.PreserveAspectFit
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.text.length > 0
                text: control.text
                color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
                font: control.font
                elide: Text.ElideRight
            }
        }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: control.checked ? Tokens.selection : "transparent"
        border.width: control.activeFocus ? 2 : 0
        border.color: Tokens.focusRing

        LfStateLayer {
            anchors.fill: parent
            control: control
            checked: control.checked
        }
    }
}
