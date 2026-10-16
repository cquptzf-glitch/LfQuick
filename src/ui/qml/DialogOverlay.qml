import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property OverlayCoordinator coordinator: null
    readonly property var request: coordinator && coordinator.dialogActive
                                   ? coordinator.activeDialog : ({})

    visible: coordinator && !coordinator.externalBackendActive && coordinator.dialogActive
    focus: visible

    function buttonLabel(buttonId) {
        if (buttonId === "ok") return qsTr("OK")
        if (buttonId === "cancel") return qsTr("Cancel")
        if (buttonId === "yes") return qsTr("Yes")
        if (buttonId === "no") return qsTr("No")
        if (buttonId === "retry") return qsTr("Retry")
        if (buttonId === "close") return qsTr("Close")
        if (buttonId === "discard") return qsTr("Discard")
        if (buttonId === "apply") return qsTr("Apply")
        return buttonId
    }

    onVisibleChanged: {
        if (visible)
            forceActiveFocus()
    }

    Keys.onEscapePressed: function(event) {
        if (root.coordinator && root.request.id)
            root.coordinator.cancelDialog(root.request.id)
        event.accepted = true
    }

    Rectangle {
        anchors.fill: parent
        color: Tokens.scrim

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
            onClicked: function(mouse) { mouse.accepted = true }
        }
    }

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: Math.min(560, Math.max(320, parent.width - Tokens.spacingLarge * 2))
        implicitHeight: contentLayout.implicitHeight + Tokens.spacingLarge * 2
        radius: Tokens.radiusMedium
        color: Tokens.surface
        border.color: Tokens.border
        border.width: 1

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            anchors.margins: Tokens.spacingLarge
            spacing: Tokens.spacingMedium

            Label {
                Layout.fillWidth: true
                visible: root.request.title && root.request.title.length > 0
                text: root.request.title || ""
                color: Tokens.textPrimary
                font.pixelSize: 20
                font.bold: true
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: root.request.message || ""
                color: Tokens.textPrimary
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Tokens.spacingSmall

                Item { Layout.fillWidth: true }

                Repeater {
                    model: root.request.buttons || []

                    delegate: Button {
                        required property string modelData
                        text: root.buttonLabel(modelData)
                        highlighted: modelData === (root.request.defaultButton || "")

                        contentItem: Label {
                            text: parent.text
                            color: modelData === (root.request.destructiveButton || "")
                                   ? Tokens.danger : Tokens.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (root.coordinator && root.request.id)
                                root.coordinator.resolveDialog(root.request.id, modelData)
                        }
                    }
                }
            }
        }
    }
}
