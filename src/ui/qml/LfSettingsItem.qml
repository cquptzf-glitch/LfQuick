import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string title: ""
    property string description: ""
    property bool divided: true
    property bool compact: false

    default property alias controlData: actionHost.data

    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: compact ? Units.sm : Units.md
    bottomPadding: topPadding

    implicitWidth: Math.max(280, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    Accessible.name: title
    Accessible.description: description

    contentItem: RowLayout {
        spacing: Units.lg

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Units.xs

            Text {
                Layout.fillWidth: true
                text: root.title
                color: root.enabled ? Tokens.textPrimary : Tokens.textDisabled
                font.pixelSize: Typography.body
                font.weight: Typography.weightMedium
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                visible: root.description.length > 0
                text: root.description
                color: root.enabled ? Tokens.textSecondary : Tokens.textDisabled
                font.pixelSize: Typography.caption
                wrapMode: Text.Wrap
            }
        }

        RowLayout {
            id: actionHost
            spacing: Units.sm
            enabled: root.enabled
        }
    }

    background: Item {
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: root.divided
            height: 1
            color: Tokens.border
        }
    }
}
