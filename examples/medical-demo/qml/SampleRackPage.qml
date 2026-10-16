import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.DataView

LfPage {
    id: root
    title: qsTr("Sample Rack")

    property bool pageActive: false
    property bool scanArmed: true
    property int previousScannerMinimumLength: 4
    property string actionStatus: qsTr("Scan or enter a rack ID to filter the rack view.")
    property var contextRows: []

    function defaultColumns() {
        return [
            { "id": "sampleId", "modelColumn": 0, "title": qsTr("Sample ID"), "width": 130, "visible": true },
            { "id": "barcode", "modelColumn": 1, "title": qsTr("Barcode"), "width": 145, "visible": true },
            { "id": "sampleType", "modelColumn": 2, "title": qsTr("Type"), "width": 100, "visible": true },
            { "id": "rack", "modelColumn": 3, "title": qsTr("Rack"), "width": 80, "visible": true },
            { "id": "position", "modelColumn": 4, "title": qsTr("Position"), "width": 85, "visible": true },
            { "id": "priority", "modelColumn": 5, "title": qsTr("Priority"), "width": 95, "visible": true },
            { "id": "status", "modelColumn": 6, "title": qsTr("Status"), "width": 120, "visible": true },
            { "id": "testCount", "modelColumn": 7, "title": qsTr("Tests"), "width": 130, "visible": true },
            { "id": "result", "modelColumn": 8, "title": qsTr("Result"), "width": 90, "visible": false },
            { "id": "createTime", "modelColumn": 9, "title": qsTr("Created"), "width": 120, "visible": false }
        ]
    }

    function updateScanner() {
        scanner.enabled = root.pageActive && root.scanArmed && rackField.activeFocus
    }

    function sourceRows(proxyRows) {
        var rows = []
        for (var i = 0; i < proxyRows.length; ++i) {
            var sourceRow = rackModel.mapRowToSource(proxyRows[i])
            if (sourceRow >= 0)
                rows.push(sourceRow)
        }
        return rows
    }

    function applyBatch(proxyRows, key, value, label) {
        var rows = root.sourceRows(proxyRows.slice(0))
        if (rows.length === 0) {
            root.actionStatus = qsTr("No rows selected.")
            return
        }
        var changed = 0
        for (var i = 0; i < rows.length; ++i) {
            if (sampleModel.setCell(rows[i], key, value))
                ++changed
        }
        root.actionStatus = qsTr("%1: updated %2 samples.").arg(label).arg(changed)
    }

    onActivated: {
        root.pageActive = true
        root.previousScannerMinimumLength = scanner.session.minimumLength
        scanner.session.minimumLength = 2
        Qt.callLater(function() {
            rackField.forceActiveFocus()
            root.updateScanner()
        })
    }

    onDeactivated: {
        root.pageActive = false
        scanner.enabled = false
        scanner.session.minimumLength = root.previousScannerMinimumLength
    }

    DataFilterProxyModel {
        id: rackModel
        sourceModel: sampleModel
        filterKey: "rack"
        filterText: rackField.text.trim()
        exactMatch: true
        caseSensitive: false
    }

    ColumnConfigStore { id: rackColumnStore }

    ColumnStateModel {
        id: rackColumns
        onStateChanged: persistRackColumns.restart()
        Component.onCompleted: {
            setColumns(root.defaultColumns())
            var saved = rackColumnStore.load("medical-sample-rack", 1, "SampleModel.v1")
            if (saved.matched)
                restoreState(saved.columns)
        }
    }

    Timer {
        id: persistRackColumns
        interval: 250
        repeat: false
        onTriggered: rackColumnStore.save("medical-sample-rack", 1, "SampleModel.v1", rackColumns.snapshot())
    }

    Component {
        id: testsProgressDelegate
        DataGridProgressDelegate {
            from: 0
            to: 8
            showText: true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Sample Rack")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }
            Label {
                text: qsTr("%1 samples in rack").arg(rackModel.count)
                color: Tokens.textSecondary
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: rackField
                Layout.preferredWidth: 160
                text: "R1"
                placeholderText: qsTr("Rack ID")
                activeFocusOnTab: true
                onActiveFocusChanged: root.updateScanner()
                onAccepted: {
                    rackGrid.clearSelection()
                    if (rackModel.count > 0)
                        rackGrid.focusFirstCell()
                }
            }
            Switch {
                text: qsTr("Scanner armed")
                checked: root.scanArmed
                onToggled: {
                    root.scanArmed = checked
                    root.updateScanner()
                }
            }
            Button {
                text: qsTr("Simulate R1")
                onClicked: scanner.simulateScan("R1")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("Queue selected")
                enabled: rackGrid.selectionController.selectedRowCount > 0
                onClicked: root.applyBatch(rackGrid.selectedRows(), "status", "Pending", qsTr("Queue"))
            }
            Button {
                text: qsTr("Mark STAT")
                enabled: rackGrid.selectionController.selectedRowCount > 0
                onClicked: root.applyBatch(rackGrid.selectedRows(), "priority", "STAT", qsTr("STAT"))
            }
            Button {
                text: qsTr("Select all in rack")
                enabled: rackModel.count > 0
                onClicked: rackGrid.selectAllRows()
            }
            Button {
                text: qsTr("Clear selection")
                enabled: rackGrid.selectionController.selectedRowCount > 0
                onClicked: rackGrid.clearSelection()
            }
            Item { Layout.fillWidth: true }
            Label {
                text: qsTr("Selected: %1").arg(rackGrid.selectionController.selectedRowCount)
                color: Tokens.textSecondary
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.actionStatus
            color: Tokens.textSecondary
            elide: Text.ElideRight
        }

        DataGrid {
            id: rackGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: rackModel
            columnState: rackColumns
            editable: false
            selectionMode: TableView.ExtendedSelection
            contextMenuEnabled: true

            cellDelegateResolver: function(columnId, logicalColumn) {
                return columnId === "testCount" ? testsProgressDelegate : null
            }

            onContextMenuRequested: function(row, modelColumn, x, y, selectedRows) {
                root.contextRows = selectedRows.slice(0)
                rackContextMenu.open()
            }
            onCellActivated: function(row, modelColumn) {
                root.actionStatus = qsTr("Current rack row %1, column %2").arg(row + 1).arg(modelColumn + 1)
            }
        }
    }

    Menu {
        id: rackContextMenu
        MenuItem {
            text: qsTr("Queue selected")
            enabled: root.contextRows.length > 0
            onTriggered: root.applyBatch(root.contextRows, "status", "Pending", qsTr("Queue"))
        }
        MenuItem {
            text: qsTr("Mark selected STAT")
            enabled: root.contextRows.length > 0
            onTriggered: root.applyBatch(root.contextRows, "priority", "STAT", qsTr("STAT"))
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Clear selection")
            onTriggered: rackGrid.clearSelection()
        }
    }

    Connections {
        target: scanner.session
        function onScanAccepted(value) {
            if (!root.pageActive)
                return
            rackField.text = value
            rackGrid.clearSelection()
            root.actionStatus = qsTr("Rack %1 loaded from scanner.").arg(value)
            rackField.forceActiveFocus()
            root.updateScanner()
        }
        function onScanRejected(rawValue, reason) {
            if (!root.pageActive)
                return
            root.actionStatus = qsTr("Scanner rejected %1: %2").arg(rawValue).arg(reason)
        }
    }

    Shortcut {
        sequence: "F6"
        onActivated: rackGrid.focusFirstCell()
    }
}
