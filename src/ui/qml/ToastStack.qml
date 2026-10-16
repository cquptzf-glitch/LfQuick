import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property OverlayCoordinator coordinator: null

    readonly property string position: coordinator ? coordinator.toastPosition : "top-right"
    readonly property bool bottomAligned: position.indexOf("bottom-") === 0
    readonly property bool centerAligned: position.indexOf("-center") >= 0

    visible: coordinator && !coordinator.externalBackendActive && coordinator.toastCount > 0

    function severityColor(severity) {
        if (severity === "success")
            return Tokens.success
        if (severity === "warning")
            return Tokens.warning
        if (severity === "error")
            return Tokens.error
        return Tokens.accent
    }

    Column {
        id: toastColumn
        width: Math.min(420, Math.max(240, root.width - Tokens.spacingLarge * 2))
        spacing: Tokens.spacingSmall

        anchors.top: root.bottomAligned ? undefined : parent.top
        anchors.bottom: root.bottomAligned ? parent.bottom : undefined
        anchors.right: root.centerAligned ? undefined : parent.right
        anchors.horizontalCenter: root.centerAligned ? parent.horizontalCenter : undefined
        anchors.margins: Tokens.spacingLarge

        Repeater {
            model: root.coordinator ? root.coordinator.toasts : []

            delegate: Rectangle {
                id: toastCard
                required property var modelData

                width: toastColumn.width
                implicitHeight: toastLayout.implicitHeight + Tokens.spacingMedium * 2
                radius: Tokens.radiusMedium
                color: Tokens.surfaceSecondary
                border.color: root.severityColor(modelData.severity)
                border.width: 1

                RowLayout {
                    id: toastLayout
                    anchors.fill: parent
                    anchors.margins: Tokens.spacingMedium
                    spacing: Tokens.spacingMedium

                    Rectangle {
                        Layout.preferredWidth: 4
                        Layout.fillHeight: true
                        radius: 2
                        color: root.severityColor(toastCard.modelData.severity)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: toastCard.modelData.message
                        color: Tokens.textPrimary
                        wrapMode: Text.Wrap
                    }

                    ToolButton {
                        text: "×"
                        Accessible.name: qsTr("Dismiss notification")
                        onClicked: {
                            if (root.coordinator)
                                root.coordinator.dismissToast(toastCard.modelData.id)
                        }
                    }
                }
            }
        }
    }
}
