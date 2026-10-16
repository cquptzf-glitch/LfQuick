import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.DataView

LfPage {
    id: root
    title: qsTr("Results")
    property string editStatus: qsTr("Double-click Result to exercise validated edit transactions")

    function defaultColumns() {
        return [
            { "id": "sampleId", "modelColumn": 0, "title": qsTr("Sample ID"), "width": 130, "visible": true },
            { "id": "assay", "modelColumn": 1, "title": qsTr("Assay"), "width": 110, "visible": true },
            { "id": "result", "modelColumn": 2, "title": qsTr("Result"), "width": 100, "visible": true },
            { "id": "unit", "modelColumn": 3, "title": qsTr("Unit"), "width": 100, "visible": true },
            { "id": "flag", "modelColumn": 4, "title": qsTr("Flag"), "width": 110, "visible": true },
            { "id": "status", "modelColumn": 5, "title": qsTr("Status"), "width": 150, "visible": true },
            { "id": "time", "modelColumn": 6, "title": qsTr("Time"), "width": 120, "visible": true }
        ]
    }

    function resetColumns() {
        resultColumnStore.clear("medical-results")
        resultColumns.setColumns(defaultColumns())
        resultsGrid.applyColumnState()
    }

    DataFilterProxyModel {
        id: filteredModel
        sourceModel: resultModel
        filterText: filterField.text
    }

    ColumnConfigStore {
        id: resultColumnStore
    }

    ColumnStateModel {
        id: resultColumns
        onStateChanged: persistColumns.restart()
        Component.onCompleted: {
            setColumns(root.defaultColumns())
            var saved = resultColumnStore.load("medical-results", 2, "ResultModel.v1")
            if (saved.matched)
                restoreState(saved.columns)
        }
    }

    Timer {
        id: persistColumns
        interval: 250
        repeat: false
        onTriggered: resultColumnStore.save("medical-results", 2, "ResultModel.v1", resultColumns.snapshot())
    }

    DataEditTransaction {
        id: resultEditTransaction
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Results")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }

            Item { Layout.fillWidth: true }

            TextField {
                id: filterField
                Layout.preferredWidth: 260
                placeholderText: qsTr("Search result")
            }

            Button {
                text: resultColumns.isModelColumnVisible(4) ? qsTr("Hide Flag") : qsTr("Show Flag")
                onClicked: {
                    resultColumns.setVisible("flag", !resultColumns.isModelColumnVisible(4))
                    resultsGrid.tableView.forceLayout()
                }
            }

            Button {
                text: qsTr("Reset Columns")
                onClicked: root.resetColumns()
            }

            Button {
                text: qsTr("Sort Result")
                onClicked: resultModel.sortByKey("result", Qt.DescendingOrder)
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.editStatus
            color: Tokens.textSecondary
            elide: Text.ElideRight
        }

        DataGrid {
            id: resultsGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: filteredModel
            columnState: resultColumns
            editTransaction: resultEditTransaction
            selectionMode: TableView.ExtendedSelection

            editValidator: function(row, modelColumn, value) {
                if (modelColumn !== 2)
                    return { "accepted": false, "message": qsTr("This demo only enables editing for the Result column") }
                var numeric = Number(value)
                if (!isFinite(numeric) || numeric < 0 || numeric > 9999)
                    return { "accepted": false, "message": qsTr("Result must be a number between 0 and 9999") }
                return { "accepted": true, "message": "" }
            }

            onEditCommitted: function(row, modelColumn, value) {
                root.editStatus = qsTr("Committed row %1 result = %2").arg(row + 1).arg(value)
            }
            onEditRejected: function(row, modelColumn, value, reason) {
                root.editStatus = qsTr("Edit rejected: %1").arg(reason)
            }
        }
    }
}
