import QtQuick

Rectangle {
    id: root

    property var coordinator

    color: "#e61c1c1f"
    border.color: "#707078"
    radius: 8
    implicitWidth: 360
    implicitHeight: 280

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 7

        Text {
            text: qsTr("Overlay Inspector")
            color: "#f1f1f3"
            font.bold: true
            font.pixelSize: 12
        }

        Text {
            width: parent.width
            text: root.coordinator
                ? qsTr("Busy: %1 · count %2 · cancelable %3")
                    .arg(root.coordinator.busy ? "yes" : "no")
                    .arg(root.coordinator.busyCount)
                    .arg(root.coordinator.busyCancelable ? "yes" : "no")
                : qsTr("No coordinator")
            color: "#cfd0d4"
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.coordinator && root.coordinator.busy
            text: qsTr("Busy message: %1 · progress %2")
                .arg(root.coordinator.busyMessage)
                .arg(Number(root.coordinator.busyProgress).toFixed(2))
            color: "#9fa1a7"
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.coordinator
                ? qsTr("Toasts: %1 · position %2")
                    .arg(root.coordinator.toastCount)
                    .arg(root.coordinator.toastPosition)
                : ""
            color: "#cfd0d4"
            font.pixelSize: 10
        }

        ListView {
            width: parent.width
            height: Math.min(100, contentHeight)
            visible: root.coordinator && root.coordinator.toastCount > 0
            model: root.coordinator ? root.coordinator.toasts : []
            clip: true
            spacing: 2

            delegate: Rectangle {
                required property int index
                required property var modelData
                width: ListView.view.width
                height: 30
                radius: 4
                color: index % 2 ? "#29292f" : "#242429"

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 7
                    anchors.rightMargin: 7
                    verticalAlignment: Text.AlignVCenter
                    text: (modelData.severity ? "[" + modelData.severity + "] " : "")
                          + (modelData.message || "")
                    color: "#aeb0b6"
                    font.pixelSize: 9
                    elide: Text.ElideRight
                }
            }
        }

        Text {
            width: parent.width
            text: root.coordinator
                ? qsTr("Dialog: %1 · pending %2 · external backend %3")
                    .arg(root.coordinator.dialogActive ? "active" : "none")
                    .arg(root.coordinator.pendingDialogCount)
                    .arg(root.coordinator.externalBackendActive ? "yes" : "no")
                : ""
            color: root.coordinator && root.coordinator.dialogActive ? "#ffcc80" : "#9fa1a7"
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.coordinator && root.coordinator.dialogActive
            text: root.coordinator.activeDialog.title + " · " + root.coordinator.activeDialog.message
            color: "#cfd0d4"
            font.pixelSize: 10
            elide: Text.ElideRight
        }
    }
}
