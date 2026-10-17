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

    property string guardDialogId: ""
    property bool compactNavigation: false
    property bool diagnosticsHudVisible: false

    readonly property var navigationDefinitions: {
        const definitions = [
            { "route": "/dashboard", "label": qsTr("Dashboard"), "page": "DashboardPage.qml", "section": "primary", "group": qsTr("Workflow") },
            { "route": "/samples", "label": qsTr("Samples"), "page": "SamplesPage.qml", "section": "primary", "group": qsTr("Workflow") },
            { "route": "/sample-register", "label": qsTr("Sample Register"), "page": "SampleRegisterPage.qml", "section": "primary", "group": qsTr("Workflow") },
            { "route": "/sample-rack", "label": qsTr("Sample Rack"), "page": "SampleRackPage.qml", "section": "primary", "group": qsTr("Workflow") },
            { "route": "/results", "label": qsTr("Results"), "page": "ResultsPage.qml", "section": "primary", "group": qsTr("Results") },
            { "route": "/results-review", "label": qsTr("Results Review"), "page": "ResultsReviewPage.qml", "section": "primary", "group": qsTr("Results"), "permissionId": "demo.results.review", "permissionContext": { "role": "operator" } },
            { "route": "/instrument", "label": qsTr("Instrument"), "page": "InstrumentPage.qml", "section": "secondary", "group": qsTr("System") },
            { "route": "/alarms", "label": qsTr("Alarms"), "page": "AlarmsPage.qml", "section": "secondary", "group": qsTr("System"), "badge": 3 },
            { "route": "/developer", "label": qsTr("Developer"), "page": "DeveloperPage.qml", "section": "footer", "group": qsTr("Tools") }
        ]

        if (chartsAvailable) {
            definitions.splice(6, 0, {
                "route": "/charts",
                "label": qsTr("Charts"),
                "page": "ChartsPage.qml",
                "section": "secondary",
                "group": qsTr("Visualization")
            })
        }
        return definitions
    }

    NavigationItemModel {
        id: navigationModel
    }

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
                visible: diagnosticsAvailable
                text: qsTr("Diagnostics")
                checked: window.diagnosticsHudVisible
                onToggled: window.diagnosticsHudVisible = checked
            }

            Switch {
                text: qsTr("Compact Nav")
                checked: window.compactNavigation
                onToggled: window.compactNavigation = checked
            }

            Switch {
                text: qsTr("Dark")
                checked: Theme.dark
                onToggled: Theme.dark = checked
            }
        }
    }

    ApplicationShell {
        id: shell
        anchors.fill: parent
        navigationModel: navigationModel
        runtime: quickRuntime
        overlayCoordinator: overlayCoordinator
        compactNavigation: window.compactNavigation
    }

    Loader {
        id: diagnosticsHudLoader
        anchors.fill: parent
        z: 100000
        active: diagnosticsAvailable && window.diagnosticsHudVisible
        source: active ? "qrc:/qt/qml/LfQuick/Diagnostics/DiagnosticsHud.qml" : ""

        onLoaded: {
            if (item)
                item.diagnosticsModel = diagnosticsModel
        }
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
        navigationModel.permissionEvaluator = permissionEvaluator
        navigationModel.setItems(window.navigationDefinitions)
        quickRuntime.host = shell.pageHost

        for (let i = 0; i < window.navigationDefinitions.length; ++i) {
            const definition = window.navigationDefinitions[i]
            quickRuntime.registerPage(definition.route, Qt.resolvedUrl(definition.page), 2)
        }

        quickRuntime.push("/dashboard")
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
