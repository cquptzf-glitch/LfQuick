import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    id: root
    title: qsTr("Hybrid Quick Page")

    property var navigationGuard
    property string guardReason: ""

    Rectangle {
        anchors.fill: parent
        color: Tokens.surface

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 64, 680)
            spacing: Tokens.spacingMedium

            Label {
                Layout.fillWidth: true
                text: qsTr("QML page inside QWidget shell")
                font.pixelSize: 28
                font.bold: true
                color: Tokens.textPrimary
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Tab order demo: QWidget QLineEdit → this QML TextField → QWidget QLineEdit. Backtab reverses the bridge.")
                wrapMode: Text.WordWrap
                color: Tokens.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }

            TextField {
                id: hybridFocusField
                objectName: "hybridFocusField"
                Layout.fillWidth: true
                placeholderText: qsTr("QML focus bridge field")
                activeFocusOnTab: false
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: guardColumn.implicitHeight + 32
                radius: Tokens.radiusMedium
                color: Tokens.surfaceSecondary
                border.color: Tokens.border

                ColumnLayout {
                    id: guardColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: Tokens.spacingMedium
                    spacing: Tokens.spacingSmall

                    Label {
                        text: qsTr("Navigation Guard")
                        font.bold: true
                        color: Tokens.textPrimary
                    }

                    CheckBox {
                        id: dirtyCheck
                        text: qsTr("Unsaved edits (confirm before leaving)")
                        onToggled: {
                            if (!root.navigationGuard)
                                return
                            root.navigationGuard.state.dirty = checked
                            root.navigationGuard.state.confirmRequired = true
                            root.navigationGuard.state.reason = checked
                                    ? qsTr("Sample registration contains unsaved changes") : ""
                        }
                    }

                    CheckBox {
                        id: busyCheck
                        text: qsTr("Maintenance/calibration running (hard block)")
                        onToggled: {
                            if (!root.navigationGuard)
                                return
                            root.navigationGuard.state.busy = checked
                            root.navigationGuard.state.confirmRequired = !checked
                            root.navigationGuard.state.reason = checked
                                    ? qsTr("Maintenance task is still running")
                                    : (dirtyCheck.checked
                                       ? qsTr("Sample registration contains unsaved changes") : "")
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.navigationGuard && root.navigationGuard.hasPending
                              ? qsTr("Pending route: %1").arg(root.navigationGuard.pendingRoute)
                              : qsTr("No pending navigation")
                        color: Tokens.textSecondary
                    }
                }
            }
        }
    }

    Connections {
        target: root.navigationGuard
        function onConfirmationRequested(fromRoute, toRoute, reason) {
            root.guardReason = reason
            guardDialog.open()
        }
        function onHardBlocked(fromRoute, toRoute, reason) {
            root.guardReason = reason
            hardBlockDialog.open()
        }
    }

    Dialog {
        id: guardDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Leave current page?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: if (root.navigationGuard) root.navigationGuard.confirmPending()
        onRejected: if (root.navigationGuard) root.navigationGuard.cancelPending()

        contentItem: Label {
            text: root.guardReason
            wrapMode: Text.WordWrap
            color: Tokens.textPrimary
            width: 360
        }
    }

    Dialog {
        id: hardBlockDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Navigation blocked")
        standardButtons: Dialog.Ok
        onAccepted: if (root.navigationGuard) root.navigationGuard.cancelPending()

        contentItem: Label {
            text: root.guardReason
            wrapMode: Text.WordWrap
            color: Tokens.textPrimary
            width: 360
        }
    }
}
