import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    id: root
    title: qsTr("Developer")

    FocusNavigator {
        id: focusNavigator
        scope: root
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        Label {
            text: qsTr("LfQuick Developer Playground")
            font.pixelSize: 28
            font.bold: true
            color: Tokens.textPrimary
        }

        Label {
            text: qsTr("Framework verification: Theme, Overlay, Focus, Scanner and DataGrid.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: Tokens.textSecondary
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: Theme.dark ? qsTr("Switch to Light") : qsTr("Switch to Dark")
                onClicked: Theme.dark = !Theme.dark
            }

            Button {
                text: qsTr("Start Busy")
                onClicked: {
                    const token = overlayCoordinator.beginBusy(qsTr("Developer operation..."), true)
                    developerBusyTimer.token = token
                    developerBusyTimer.restart()
                }
            }

            Button {
                text: qsTr("Simulate Scanner")
                onClicked: scanner.simulateScan("DEV-SCAN-20261016")
            }

            Button {
                text: qsTr("Next Focus")
                onClicked: focusNavigator.moveNext()
            }
        }

        FocusDebugPanel {
            Layout.fillWidth: true
            navigator: focusNavigator
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            radius: Tokens.radiusMedium
            color: Tokens.surfaceSecondary
            border.color: Tokens.border

            Column {
                anchors.centerIn: parent
                spacing: Tokens.spacingSmall

                Label {
                    text: qsTr("Runtime metrics")
                    font.bold: true
                    color: Tokens.textPrimary
                }

                Label {
                    text: qsTr("Samples: %1 | Results: %2 | Scanner: %3 | InputRouter: %4")
                        .arg(sampleModel.count)
                        .arg(resultModel.count)
                        .arg(scanner.enabled ? qsTr("armed") : qsTr("off"))
                        .arg(inputRouter.installed ? qsTr("installed") : qsTr("off"))
                    color: Tokens.textSecondary
                }

                Label {
                    text: qsTr("Focus items: %1").arg(focusNavigator.focusChain().length)
                    color: Tokens.textSecondary
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: Tokens.radiusMedium
            color: Tokens.surfaceSecondary
            border.color: Tokens.border

            Column {
                anchors.centerIn: parent
                spacing: Tokens.spacingSmall

                Label { text: qsTr("Semantic token preview"); color: Tokens.textPrimary }

                Row {
                    spacing: Tokens.spacingSmall

                    Repeater {
                        model: [Tokens.accent, Tokens.success, Tokens.warning, Tokens.error, Tokens.critical]

                        Rectangle {
                            required property color modelData
                            width: 48
                            height: 24
                            radius: 4
                            color: modelData
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: developerBusyTimer
        property string token: ""
        interval: 1600
        repeat: false
        onTriggered: {
            if (token.length > 0) {
                overlayCoordinator.endBusy(token)
                token = ""
            }
        }
    }
}
