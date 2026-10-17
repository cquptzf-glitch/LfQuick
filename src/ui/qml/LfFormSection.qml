import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string title: ""
    property string description: ""
    property bool framed: false
    property bool showDivider: true

    default property alias contentData: body.data

    leftPadding: framed ? Units.lg : 0
    rightPadding: leftPadding
    topPadding: framed ? Units.lg : 0
    bottomPadding: topPadding

    implicitWidth: Math.max(280, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    contentItem: ColumnLayout {
        spacing: Units.md

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.title.length > 0 || root.description.length > 0
            spacing: Units.xs

            Text {
                Layout.fillWidth: true
                visible: root.title.length > 0
                text: root.title
                color: Tokens.textPrimary
                font.pixelSize: Typography.subtitle
                font.weight: Typography.weightSemibold
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                visible: root.description.length > 0
                text: root.description
                color: Tokens.textSecondary
                font.pixelSize: Typography.body
                wrapMode: Text.Wrap
            }
        }

        LfDivider {
            Layout.fillWidth: true
            visible: root.showDivider && (root.title.length > 0 || root.description.length > 0)
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: Units.md
        }
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: root.framed ? Tokens.surfaceSecondary : "transparent"
        border.width: root.framed ? 1 : 0
        border.color: Tokens.border
    }
}
