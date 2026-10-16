import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.DataView

LfPage {
    id: root
    title: qsTr("Samples")

    property bool scanMode: false
    property string busyToken: ""

    DataFilterProxyModel {
        id: filteredModel
        sourceModel: sampleModel
        filterText: filterField.text
    }

    FocusNavigator {
        id: focusNavigator
        scope: root
    }

    onDeactivated: scanner.enabled = false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Samples")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("%1 rows").arg(sampleModel.count)
                color: Tokens.textSecondary
            }
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: filterField
                Layout.preferredWidth: 260
                placeholderText: qsTr("Search sample / barcode / status")
            }

            TextField {
                id: barcodeField
                Layout.preferredWidth: 240
                placeholderText: qsTr("Scan barcode")
                activeFocusOnTab: true
                onActiveFocusChanged: {
                    scanner.enabled = root.scanMode && activeFocus
                }
                onAccepted: {
                    if (text.length > 0)
                        scanner.simulateScan(text)
                }
            }

            Switch {
                text: qsTr("Scanner")
                checked: root.scanMode
                onToggled: {
                    root.scanMode = checked
                    scanner.enabled = checked && barcodeField.activeFocus
                }
            }

            Button {
                text: qsTr("Simulate Scan")
                onClicked: scanner.simulateScan("DEMO-SCAN-001")
            }

            Button {
                text: qsTr("Busy")
                onClicked: {
                    if (root.busyToken.length > 0)
                        return
                    root.busyToken = overlayCoordinator.beginBusy(qsTr("Loading samples..."), false)
                    busyTimer.restart()
                }
            }

            Button {
                text: qsTr("Sort ID")
                onClicked: sampleModel.sortByKey("sampleId", Qt.AscendingOrder)
            }
        }

        DataGrid {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: filteredModel
            columnWidths: [130, 150, 100, 80, 80, 100, 120, 70, 100, 130]
            selectionMode: TableView.ExtendedSelection
        }
    }

    Timer {
        id: busyTimer
        interval: 1200
        repeat: false
        onTriggered: {
            if (root.busyToken.length > 0) {
                overlayCoordinator.endBusy(root.busyToken)
                root.busyToken = ""
            }
        }
    }

    Connections {
        target: scanner.session
        function onScanAccepted(value) {
            barcodeField.text = value
            root.scanMode = false
            scanner.enabled = false
            sampleModel.appendRow({
                sampleId: "SCAN-" + value,
                barcode: value,
                sampleType: "Serum",
                rack: "SCAN",
                position: 1,
                priority: "STAT",
                status: "Pending",
                testCount: 1,
                result: "",
                createTime: "Now"
            })
            barcodeField.forceActiveFocus()
        }

        function onScanRejected(rawValue, reason) {
            console.warn("Scanner rejected:", rawValue, reason)
        }
    }

    Shortcut {
        sequence: "F2"
        onActivated: focusNavigator.moveNext()
    }
}
