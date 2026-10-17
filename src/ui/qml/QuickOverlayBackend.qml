import QtQuick

Item {
    id: root

    property OverlayCoordinator coordinator: null

    readonly property alias stack: stack
    readonly property alias popupLayer: stack.popupLayer
    readonly property alias dropdownLayer: stack.dropdownLayer
    readonly property alias tooltipLayer: stack.tooltipLayer
    readonly property alias toastLayer: stack.toastLayer
    readonly property alias modalLayer: stack.modalLayer
    readonly property alias dialogLayer: stack.dialogLayer
    readonly property alias busyLayer: stack.busyLayer
    readonly property alias criticalLayer: stack.criticalLayer

    OverlayStack {
        id: stack
        anchors.fill: parent
    }

    ToastStack {
        parent: stack.toastLayer
        anchors.fill: parent
        coordinator: root.coordinator
    }

    DialogOverlay {
        parent: stack.dialogLayer
        anchors.fill: parent
        coordinator: root.coordinator
    }

    BusyOverlay {
        parent: stack.busyLayer
        anchors.fill: parent
        active: root.coordinator ? root.coordinator.busy : false
        message: root.coordinator ? root.coordinator.busyMessage : ""
        progress: root.coordinator ? root.coordinator.busyProgress : 0
        cancelable: root.coordinator ? root.coordinator.busyCancelable : false
        onCancelRequested: if (root.coordinator) root.coordinator.cancelTopBusy()
    }
}
