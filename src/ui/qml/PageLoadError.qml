import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

LfPage {
    id: root

    property string route: ""
    property string errorMessage: ""

    title: qsTr("Page load failed")

    Rectangle {
        anchors.fill: parent
        color: Tokens.surface

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - Tokens.spacingLarge * 2, 720)
            spacing: Tokens.spacingMedium

            Label {
                Layout.fillWidth: true
                text: qsTr("This page could not be loaded")
                font.pixelSize: 24
                font.bold: true
                color: Tokens.error
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                visible: root.route.length > 0
                text: root.route
                color: Tokens.textPrimary
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                text: root.errorMessage.length > 0
                      ? root.errorMessage
                      : qsTr("No additional QML error information is available.")
                wrapMode: Text.WordWrap
                color: Tokens.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
