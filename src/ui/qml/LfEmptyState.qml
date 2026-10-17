import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string symbol: "—"
    property string title: ""
    property string message: ""
    property string actionText: ""
    property string secondaryActionText: ""

    signal actionTriggered()
    signal secondaryActionTriggered()

    leftPadding: Units.xl
    rightPadding: Units.xl
    topPadding: Units.xl
    bottomPadding: Units.xl

    implicitWidth: Math.max(280, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    contentItem: ColumnLayout {
        spacing: Units.md

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: Theme.density === Theme.Touch ? 64 : 52
            height: width
            radius: width / 2
            color: Tokens.surfaceTertiary
            border.width: 1
            border.color: Tokens.border

            Text {
                anchors.centerIn: parent
                text: root.symbol
                color: Tokens.textSecondary
                font.pixelSize: Typography.titleLarge
                font.weight: Typography.weightSemibold
            }
        }

        Text {
            Layout.fillWidth: true
            visible: root.title.length > 0
            text: root.title
            color: Tokens.textPrimary
            font.pixelSize: Typography.subtitle
            font.weight: Typography.weightSemibold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        Text {
            Layout.fillWidth: true
            visible: root.message.length > 0
            text: root.message
            color: Tokens.textSecondary
            font.pixelSize: Typography.body
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Units.sm
            visible: root.actionText.length > 0 || root.secondaryActionText.length > 0

            LfButton {
                visible: root.actionText.length > 0
                text: root.actionText
                highlighted: true
                onClicked: root.actionTriggered()
            }

            LfButton {
                visible: root.secondaryActionText.length > 0
                text: root.secondaryActionText
                onClicked: root.secondaryActionTriggered()
            }
        }
    }

    background: Item { }
}
