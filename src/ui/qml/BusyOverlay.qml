import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool active: false
    property string message: ""
    property real progress: 0
    property bool cancelable: false

    signal cancelRequested()

    visible: active
    enabled: active

    Rectangle {
        anchors.fill: parent
        color: Tokens.scrim

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onPressed: mouse.accepted = true
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Tokens.spacingLarge * 2, 420)
        spacing: Tokens.spacingMedium

        BusyIndicator {
            running: root.active
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            visible: root.message.length > 0
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: root.message
            color: Tokens.onScrim
        }

        ProgressBar {
            visible: root.progress > 0 && root.progress < 1
            width: parent.width
            value: root.progress
        }

        Button {
            visible: root.cancelable
            text: qsTr("Cancel")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: root.cancelRequested()
        }
    }
}
