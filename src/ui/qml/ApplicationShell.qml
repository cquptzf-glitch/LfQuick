import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property var navigationModel
    property var navigationController
    property var runtime
    property var overlayCoordinator
    property bool compactNavigation: false
    property bool responsiveNavigation: false
    property bool hideNavigationWhenNarrow: false
    property bool showBackButton: true

    property alias pageHost: pageHost
    property alias navigationView: navigationView
    property alias navigationPolicy: responsivePolicy
    readonly property var effectiveNavigationController: navigationController || internalNavigationController
    readonly property bool navigationVisible: !responsiveNavigation || responsivePolicy.visible
    readonly property bool navigationCompact: responsiveNavigation ? responsivePolicy.compact : compactNavigation

    function syncControllerModel() {
        if (effectiveNavigationController && effectiveNavigationController.model !== root.navigationModel)
            effectiveNavigationController.model = root.navigationModel
    }

    function syncNavigationState() {
        if (!effectiveNavigationController || !root.runtime)
            return
        effectiveNavigationController.syncNavigationState(
            root.runtime.currentRoute,
            root.runtime.canGoBack,
            root.runtime.canGoForward)
    }

    NavigationController {
        id: internalNavigationController
        model: root.navigationModel
    }

    NavigationResponsivePolicy {
        id: responsivePolicy
        width: root.width
        allowHidden: root.hideNavigationWhenNarrow
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationView {
            id: navigationView
            visible: root.navigationVisible
            Layout.preferredWidth: visible ? navigationWidth : 0
            Layout.fillHeight: true
            model: root.navigationModel
            controller: root.effectiveNavigationController
            compact: root.navigationCompact
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

    Connections {
        target: root.effectiveNavigationController
        function onNavigateRequested(route, parameters) {
            if (root.runtime)
                root.runtime.push(route, parameters)
            else
                root.navigationView.routeRequested(route, parameters)
        }
        function onBackRequested() {
            if (root.runtime)
                root.runtime.back()
            else
                root.navigationView.backRequested()
        }
        function onForwardRequested() {
            if (root.runtime)
                root.runtime.forward()
        }
    }

    Connections {
        target: root.runtime
        function onCurrentRouteChanged() { root.syncNavigationState() }
        function onNavigationChanged() { root.syncNavigationState() }
    }

    onNavigationControllerChanged: {
        syncControllerModel()
        syncNavigationState()
    }
    onNavigationModelChanged: syncControllerModel()
    onRuntimeChanged: syncNavigationState()

    Component.onCompleted: {
        syncControllerModel()
        syncNavigationState()
    }
}
