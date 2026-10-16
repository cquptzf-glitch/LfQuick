import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool active: false
    property string message: ""
    visible: active
    enabled: active

    Rectangle {
        anchors.fill: parent
        color: Tokens.scrim
    }

    Column {
        anchors.centerIn: parent
        spacing: Tokens.spacingMedium

        BusyIndicator {
            running: root.active
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            visible: root.message.length > 0
            text: root.message
            color: Tokens.onScrim
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
