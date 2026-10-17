import QtQuick

Item {
    id: root

    default property alias contentData: content.data
    property OverlayCoordinator coordinator: null
    readonly property alias contentItem: content
    readonly property alias overlayBackend: overlayBackend

    Item {
        id: content
        anchors.fill: parent
    }

    QuickOverlayBackend {
        id: overlayBackend
        anchors.fill: parent
        coordinator: root.coordinator
        z: 100000
    }
}
