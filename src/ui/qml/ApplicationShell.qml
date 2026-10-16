import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property var navigationModel
    property var runtime
    property var overlayCoordinator
    property bool compactNavigation: false
    property bool showBackButton: true

    property alias pageHost: pageHost
    property alias navigationView: navigationView

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationView {
            id: navigationView
            Layout.preferredWidth: navigationWidth
            Layout.fillHeight: true
            model: root.navigationModel
            runtime: root.runtime
            compact: root.compactNavigation
            showBackButton: root.showBackButton
        }

        Item {
            id: pageHost
            objectName: "applicationShellPageHost"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
        }
    }

    OverlayHost {
        anchors.fill: parent
        coordinator: root.overlayCoordinator
    }
}
