import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

ApplicationWindow {
    id: window
    width: 1440
    height: 900
    visible: true
    title: qsTr("LfQuick Medical Demo")
    color: Tokens.surface

    property int currentPage: 0
    property string guardDialogId: ""
    readonly property var routes: [
        "/dashboard", "/samples", "/sample-register", "/sample-rack",
        "/results", "/results-review", "/instrument", "/alarms", "/developer"
    ]
    readonly property var pageFiles: [
        "DashboardPage.qml", "SamplesPage.qml", "SampleRegisterPage.qml", "SampleRackPage.qml",
        "ResultsPage.qml", "ResultsReviewPage.qml", "InstrumentPage.qml", "AlarmsPage.qml", "DeveloperPage.qml"
    ]

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            Label {
                text: "LfQuick Medical Demo"
                font.bold: true
                color: Tokens.textPrimary
            }
            Item { Layout.fillWidth: true }

            Label {
                text: quickRuntime.currentRoute.length > 0
                    ? qsTr("Route: %1").arg(quickRuntime.currentRoute)
                    : qsTr("Route: -")
                color: Tokens.textSecondary
            }

            Label {
                text: scanner && scanner.enabled ? qsTr("Scanner: Armed") : qsTr("Scanner: Off")
                color: scanner && scanner.enabled ? Tokens.success : Tokens.textSecondary
            }

            Label {
                text: qsTr("Instrument: Ready")
                color: Tokens.success
            }

            Switch {
                text: qsTr("Dark")
                checked: Theme.dark
                onToggled: Theme.dark = checked
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationRail {
            Layout.preferredWidth: 210
            Layout.fillHeight: true
            currentIndex: window.currentPage
            onActivated: quickRuntime.push(window.routes[index])
        }

        Item {
            id: pageHost
            objectName: "pageHost"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
        }
    }

    OverlayHost {
        anchors.fill: parent
        coordinator: overlayCoordinator
    }

    function indexForRoute(route) {
        const index = window.routes.indexOf(route)
        return index < 0 ? 0 : index
    }

    function requestGuardDialog(kind, title, reason, buttons, defaultButton) {
        if (window.guardDialogId.length > 0)
            return
        window.guardDialogId = overlayCoordinator.requestDialog(title, reason, {
            "buttons": buttons,
            "defaultButton": defaultButton,
            "context": { "kind": kind }
        })
    }

    Component.onCompleted: {
        quickRuntime.setHost(pageHost)

        for (let i = 0; i < window.routes.length; ++i) {
            const route = window.routes[i]
            quickRuntime.registerPage(route, Qt.resolvedUrl(window.pageFiles[i]), 2)
        }

        quickRuntime.push("/dashboard")
    }

    Connections {
        target: quickRuntime
        function onCurrentRouteChanged() {
            window.currentPage = window.indexForRoute(quickRuntime.currentRoute)
        }
    }

    Connections {
        target: quickRuntime.navigationGuard
        function onConfirmationRequested(fromRoute, toRoute, reason) {
            window.requestGuardDialog(
                "guard-confirm",
                qsTr("Leave current page?"),
                reason,
                ["ok", "cancel"],
                "cancel")
        }
        function onHardBlocked(fromRoute, toRoute, reason) {
            window.requestGuardDialog(
                "guard-block",
                qsTr("Navigation blocked"),
                reason,
                ["ok"],
                "ok")
        }
    }

    Connections {
        target: overlayCoordinator
        function onDialogResolved(requestId, buttonId, context) {
            if (requestId !== window.guardDialogId)
                return
            window.guardDialogId = ""

            if (context.kind === "guard-confirm") {
                if (buttonId === "ok")
                    quickRuntime.navigationGuard.confirmPending()
                else
                    quickRuntime.navigationGuard.cancelPending()
            } else if (context.kind === "guard-block") {
                quickRuntime.navigationGuard.cancelPending()
            }
        }
    }
}
