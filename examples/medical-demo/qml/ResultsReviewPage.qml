import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.DataView

LfPage {
    id: root
    title: qsTr("Results Review")

    property string roleId: "reviewer"
    property string operationStatus: qsTr("Select rows or edit Result / Flag / Status")
    property bool pageActive: false
    readonly property bool canReview: permissionEvaluator
        ? permissionEvaluator.isEditable("demo.results.review", { "role": root.roleId })
        : false

    function defaultColumns() {
        return [
            { "id": "sampleId", "modelColumn": 0, "title": qsTr("Sample ID"), "width": 130, "visible": true },
            { "id": "assay", "modelColumn": 1, "title": qsTr("Assay"), "width": 100, "visible": true },
            { "id": "result", "modelColumn": 2, "title": qsTr("Result"), "width": 150, "visible": true },
            { "id": "unit", "modelColumn": 3, "title": qsTr("Unit"), "width": 100, "visible": true },
            { "id": "flag", "modelColumn": 4, "title": qsTr("Flag"), "width": 120, "visible": true },
            { "id": "status", "modelColumn": 5, "title": qsTr("Status"), "width": 160, "visible": true },
            { "id": "time", "modelColumn": 6, "title": qsTr("Time"), "width": 110, "visible": true }
        ]
    }

    function permissionDeniedMessage() {
        return root.roleId === "operator"
            ? qsTr("Operator role is read-only for review actions")
            : qsTr("Current role cannot review results")
    }

    function updateGuard() {
        if (!root.pageActive || !quickRuntime || !quickRuntime.navigationGuard)
            return
        var state = quickRuntime.navigationGuard.state
        state.enabled = true
        if (saveNoteTimer.running) {
            state.busy = true
            state.dirty = false
            state.confirmRequired = false
            state.reason = qsTr("Review note is being saved")
        } else {
            state.busy = false
            state.dirty = reviewNote.text.trim().length > 0
            state.confirmRequired = true
            state.reason = state.dirty ? qsTr("Review note has not been saved") : ""
        }
    }

    function clearGuard() {
        if (!quickRuntime || !quickRuntime.navigationGuard)
            return
        var state = quickRuntime.navigationGuard.state
        state.dirty = false
        state.busy = false
        state.confirmRequired = true
        state.reason = ""
    }

    function sourceRowsFor(proxyRows) {
        var result = []
        for (var i = 0; i < proxyRows.length; ++i) {
            var sourceRow = reviewModel.mapRowToSource(Number(proxyRows[i]))
            if (sourceRow >= 0 && result.indexOf(sourceRow) < 0)
                result.push(sourceRow)
        }
        return result
    }

    function applyBatchStatus(proxyRows, status) {
        if (!root.canReview) {
            root.operationStatus = root.permissionDeniedMessage()
            return false
        }
        var sourceRows = root.sourceRowsFor(proxyRows)
        if (sourceRows.length === 0) {
            root.operationStatus = qsTr("No valid rows selected")
            return false
        }
        var changed = 0
        for (var i = 0; i < sourceRows.length; ++i) {
            if (resultModel.setCell(sourceRows[i], "status", status))
                ++changed
        }
        resultsGrid.clearSelection()
        root.operationStatus = qsTr("Updated %1 result rows to %2").arg(changed).arg(status)
        return changed > 0
    }

    function beginSaveNote() {
        if (!root.canReview) {
            root.operationStatus = root.permissionDeniedMessage()
            return
        }
        if (reviewNote.text.trim().length === 0)
            return
        saveNoteTimer.busyToken = overlayCoordinator.beginBusy(qsTr("Saving result review note"), false)
        saveNoteTimer.restart()
        root.updateGuard()
    }

    onActivated: {
        root.pageActive = true
        root.updateGuard()
    }
    onDeactivated: {
        root.pageActive = false
        root.clearGuard()
    }

    DataFilterProxyModel {
        id: reviewModel
        sourceModel: resultModel
        filterText: filterField.text
    }

    ColumnConfigStore { id: columnStore }

    ColumnStateModel {
        id: reviewColumns
        onStateChanged: persistColumns.restart()
        Component.onCompleted: {
            setColumns(root.defaultColumns())
            var saved = columnStore.load("medical-results-review", 1, "ResultModel.v1")
            if (saved.matched)
                restoreState(saved.columns)
        }
    }

    Timer {
        id: persistColumns
        interval: 250
        repeat: false
        onTriggered: columnStore.save("medical-results-review", 1,
                                      "ResultModel.v1", reviewColumns.snapshot())
    }

    DataEditTransaction { id: editTransaction }

    Timer {
        id: saveNoteTimer
        property string busyToken: ""
        interval: 650
        repeat: false
        onTriggered: {
            if (busyToken.length > 0)
                overlayCoordinator.endBusy(busyToken)
            busyToken = ""
            reviewNote.text = ""
            root.operationStatus = qsTr("Review note saved")
            root.updateGuard()
        }
    }

    Component { id: resultProgressCell; DataGridProgressDelegate { from: 0; to: 120; showText: true } }
    Component { id: comboCell; DataGridComboDelegate {} }
    Component { id: comboEditor; DataGridComboEditorDelegate {} }

    Menu {
        id: reviewMenu
        parent: resultsGrid
        property var rows: []

        MenuItem {
            text: qsTr("Mark selected Reviewed")
            enabled: root.canReview && reviewMenu.rows.length > 0
            onTriggered: root.applyBatchStatus(reviewMenu.rows, "Reviewed")
        }
        MenuItem {
            text: qsTr("Hold selected")
            enabled: root.canReview && reviewMenu.rows.length > 0
            onTriggered: root.applyBatchStatus(reviewMenu.rows, "Held")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Results Review")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }

            Label {
                text: qsTr("Role")
                color: Tokens.textSecondary
            }

            ComboBox {
                id: roleSelector
                model: [qsTr("Reviewer"), qsTr("Operator"), qsTr("Guest")]
                onCurrentIndexChanged: {
                    root.roleId = ["reviewer", "operator", "guest"][currentIndex]
                    root.operationStatus = root.canReview
                        ? qsTr("Review permission granted")
                        : root.permissionDeniedMessage()
                }
            }

            Label {
                text: root.canReview ? qsTr("Editable") : qsTr("Read-only")
                color: root.canReview ? Tokens.success : Tokens.textSecondary
            }

            Item { Layout.fillWidth: true }

            TextField {
                id: filterField
                Layout.preferredWidth: 240
                placeholderText: qsTr("Filter results")
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Selected: %1").arg(resultsGrid.selectionController.selectedRowCount)
                color: Tokens.textSecondary
            }

            Button {
                text: qsTr("Select All Visible")
                onClicked: resultsGrid.selectAllRows()
            }

            Button {
                text: qsTr("Mark Reviewed")
                enabled: root.canReview && resultsGrid.selectionController.selectedRowCount > 0
                onClicked: resultsGrid.requestBatchAction("review")
            }

            Button {
                text: qsTr("Hold")
                enabled: root.canReview && resultsGrid.selectionController.selectedRowCount > 0
                onClicked: resultsGrid.requestBatchAction("hold")
            }

            Button {
                text: qsTr("Clear Selection")
                enabled: resultsGrid.selectionController.selectedRowCount > 0
                onClicked: resultsGrid.clearSelection()
            }

            Item { Layout.fillWidth: true }

            Label {
                Layout.maximumWidth: 480
                text: root.operationStatus
                color: Tokens.textSecondary
                elide: Text.ElideRight
            }
        }

        DataGrid {
            id: resultsGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: reviewModel
            columnState: reviewColumns
            editTransaction: editTransaction
            editable: root.canReview
            selectionMode: TableView.ExtendedSelection

            cellDelegateResolver: function(columnId, modelColumn) {
                if (columnId === "result")
                    return resultProgressCell
                if (columnId === "flag" || columnId === "status")
                    return comboCell
                return null
            }

            editorDelegateResolver: function(columnId, modelColumn) {
                if (columnId === "flag" || columnId === "status")
                    return comboEditor
                return null
            }

            editorOptionsResolver: function(columnId, modelColumn) {
                if (columnId === "flag")
                    return ["Normal", "High", "Low", "Critical"]
                if (columnId === "status")
                    return ["Pending Review", "Reviewed", "Held"]
                return []
            }

            editValidator: function(row, modelColumn, value) {
                if (!root.canReview)
                    return { "accepted": false, "message": root.permissionDeniedMessage() }
                if (modelColumn === 2) {
                    var numeric = Number(value)
                    if (!isFinite(numeric) || numeric < 0 || numeric > 9999)
                        return { "accepted": false, "message": qsTr("Result must be between 0 and 9999") }
                    return { "accepted": true, "message": "" }
                }
                if (modelColumn === 4) {
                    var flags = ["Normal", "High", "Low", "Critical"]
                    return flags.indexOf(String(value)) >= 0
                        ? { "accepted": true, "message": "" }
                        : { "accepted": false, "message": qsTr("Unsupported result flag") }
                }
                if (modelColumn === 5) {
                    var states = ["Pending Review", "Reviewed", "Held"]
                    return states.indexOf(String(value)) >= 0
                        ? { "accepted": true, "message": "" }
                        : { "accepted": false, "message": qsTr("Unsupported review status") }
                }
                return { "accepted": false, "message": qsTr("This column is read-only") }
            }

            onEditCommitted: function(row, modelColumn, value) {
                root.operationStatus = qsTr("Committed row %1, column %2").arg(row + 1).arg(modelColumn + 1)
            }
            onEditRejected: function(row, modelColumn, value, reason) {
                root.operationStatus = qsTr("Edit rejected: %1").arg(reason)
            }
            onBatchActionRequested: function(actionId, selectedRows) {
                if (actionId === "review")
                    root.applyBatchStatus(selectedRows, "Reviewed")
                else if (actionId === "hold")
                    root.applyBatchStatus(selectedRows, "Held")
            }
            onContextMenuRequested: function(row, modelColumn, x, y, selectedRows) {
                reviewMenu.rows = selectedRows
                reviewMenu.x = x
                reviewMenu.y = y
                reviewMenu.open()
            }
        }

        Frame {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                TextArea {
                    id: reviewNote
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    placeholderText: qsTr("Type an audit/review note. Leaving with unsaved text exercises NavigationGuard.")
                    wrapMode: TextEdit.Wrap
                    enabled: root.canReview && !saveNoteTimer.running
                    onTextChanged: root.updateGuard()
                }

                Button {
                    text: saveNoteTimer.running ? qsTr("Saving…") : qsTr("Save Note")
                    enabled: root.canReview && reviewNote.text.trim().length > 0 && !saveNoteTimer.running
                    onClicked: root.beginSaveNote()
                }
            }
        }
    }
}
