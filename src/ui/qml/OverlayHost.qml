import QtQuick

Item {
    id: root

    property OverlayCoordinator coordinator: null

    readonly property alias popupLayer: popupLayer
    readonly property alias dropdownLayer: dropdownLayer
    readonly property alias tooltipLayer: tooltipLayer
    readonly property alias toastLayer: toastLayer
    readonly property alias modalLayer: modalLayer
    readonly property alias dialogLayer: dialogLayer
    readonly property alias busyLayer: busyLayer
    readonly property alias criticalLayer: criticalLayer

    Item { id: popupLayer; anchors.fill: parent; z: 100 }
    Item { id: dropdownLayer; anchors.fill: parent; z: 200 }
    Item { id: tooltipLayer; anchors.fill: parent; z: 300 }
    Item { id: modalLayer; anchors.fill: parent; z: 500 }
    Item { id: dialogLayer; anchors.fill: parent; z: 600 }

    Item {
        id: busyLayer
        anchors.fill: parent
        z: 700

        BusyOverlay {
            anchors.fill: parent
            active: root.coordinator ? root.coordinator.busy : false
            message: root.coordinator ? root.coordinator.busyMessage : ""
            progress: root.coordinator ? root.coordinator.busyProgress : 0
            cancelable: root.coordinator ? root.coordinator.busyCancelable : false
            onCancelRequested: if (root.coordinator) root.coordinator.cancelTopBusy()
        }
    }

    Item { id: criticalLayer; anchors.fill: parent; z: 800 }
    Item { id: toastLayer; anchors.fill: parent; z: 400 }
}
