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
    property string guardReason: ""
    readonly property var routes: [
        "/dashboard", "/samples", "/results", "/results-review",
        "/instrument", "/alarms", "/developer"
    ]
    readonly property var pageFiles: [
        "DashboardPage.qml", "SamplesPage.qml", "ResultsPage.qml", "ResultsReviewPage.qml",
        "InstrumentPage.qml", "AlarmsPage.qml", "DeveloperPage.qml"
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
            Layout.preferredWidth: 190
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

    Dialog {
        id: guardDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Leave current page?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: quickRuntime.navigationGuard.confirmPending()
        onRejected: quickRuntime.navigationGuard.cancelPending()

        contentItem: Label {
            width: 380
            text: window.guardReason
            wrapMode: Text.WordWrap
            color: Tokens.textPrimary
        }
    }

    Dialog {
        id: hardBlockDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Navigation blocked")
        standardButtons: Dialog.Ok
        onAccepted: quickRuntime.navigationGuard.cancelPending()

        contentItem: Label {
            width: 380
            text: window.guardReason
            wrapMode: Text.WordWrap
            color: Tokens.textPrimary
        }
    }

    function indexForRoute(route) {
        const index = window.routes.indexOf(route)
        return index < 0 ? 0 : index
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
            window.guardReason = reason
            guardDialog.open()
        }
        function onHardBlocked(fromRoute, toRoute, reason) {
            window.guardReason = reason
            hardBlockDialog.open()
        }
    }
}
