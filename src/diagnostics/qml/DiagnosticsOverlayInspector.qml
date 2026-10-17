import QtQuick

Rectangle {
    id: root

    property var coordinator

    color: DiagnosticsTokens.panel
    border.color: DiagnosticsTokens.panelBorder
    radius: 8
    implicitWidth: 360
    implicitHeight: 280

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 7

        Text {
            text: qsTr("Overlay Inspector")
            color: DiagnosticsTokens.textStrong
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
            color: DiagnosticsTokens.textSecondary
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.coordinator && root.coordinator.busy
            text: qsTr("Busy message: %1 · progress %2")
                .arg(root.coordinator.busyMessage)
                .arg(Number(root.coordinator.busyProgress).toFixed(2))
            color: DiagnosticsTokens.textDim
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
            color: DiagnosticsTokens.textSecondary
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
                color: index % 2
                       ? DiagnosticsTokens.rowOdd
                       : DiagnosticsTokens.rowEven

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 7
                    anchors.rightMargin: 7
                    verticalAlignment: Text.AlignVCenter
                    text: (modelData.severity ? "[" + modelData.severity + "] " : "")
                          + (modelData.message || "")
                    color: DiagnosticsTokens.textMuted
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
            color: root.coordinator && root.coordinator.dialogActive
                   ? DiagnosticsTokens.warningSoft
                   : DiagnosticsTokens.textDim
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.coordinator && root.coordinator.dialogActive
            text: root.coordinator.activeDialog.title + " · " + root.coordinator.activeDialog.message
            color: DiagnosticsTokens.textSecondary
            font.pixelSize: 10
            elide: Text.ElideRight
        }
    }
}
