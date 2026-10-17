import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Dialog {
    id: root

    property string message: ""
    property string acceptText: qsTr("OK")
    property string rejectText: qsTr("Cancel")
    property bool showRejectButton: true

    modal: true
    focus: true
    padding: 18
    closePolicy: Controls.Popup.CloseOnEscape

    background: Rectangle {
        radius: BrandTokens.radius
        color: BrandTokens.surface
        border.color: BrandTokens.border
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            Layout.fillWidth: true
            visible: root.title.length > 0
            text: root.title
            color: BrandTokens.text
            font.pixelSize: BrandTokens.fontSize + 3
            font.bold: true
            wrapMode: Text.WordWrap
        }

        Text {
            Layout.fillWidth: true
            visible: root.message.length > 0
            text: root.message
            color: BrandTokens.textSecondary
            font.pixelSize: BrandTokens.fontSize
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: BrandTokens.spacing
            Item { Layout.fillWidth: true }

            BrandButton {
                visible: root.showRejectButton
                text: root.rejectText
                onClicked: root.reject()
            }

            BrandButton {
                text: root.acceptText
                highlighted: true
                onClicked: root.accept()
            }
        }
    }

    footer: null
    header: null
}
