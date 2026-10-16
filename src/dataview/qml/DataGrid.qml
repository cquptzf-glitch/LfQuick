import QtQuick
import QtQuick.Controls
import QtQml.Models
import LfQuick

Control {
    id: root

    property var model: null
    property int rowHeight: Tokens.controlHeight
    property int headerHeight: Tokens.controlHeight
    property int minimumColumnWidth: 80
    property var columnWidths: []
    property var columnState: null
    property var editTransaction: null
    property var editValidator: null
    property bool movableColumns: true
    property bool resizableColumns: true
    property bool editable: true
    property bool alternatingRows: true
    property bool selectRows: true
    property int selectionMode: TableView.ExtendedSelection

    // Compatibility: cellDelegate applies one component to every column.
    // Typed columns should use the resolver contracts below. Each resolver
    // receives (stableColumnId, logicalModelColumn) and returns a Component.
    property Component cellDelegate: null
    property var cellDelegateResolver: null
    property var editorDelegateResolver: null
    property var editorOptionsResolver: null

    property var nextGrid: null
    property var previousGrid: null
    property bool crossGridNavigation: true
    property bool contextMenuEnabled: true

    readonly property alias tableView: table
    readonly property alias selectionModel: selectionModel
    readonly property alias selectionController: selectionController
    property bool _restoringColumnState: false
    property var _visualOrder: []

    signal editCommitted(int row, int modelColumn, var value)
    signal editRejected(int row, int modelColumn, var value, string reason)
    signal contextMenuRequested(int row, int modelColumn, real x, real y, var selectedRows)
    signal batchActionRequested(string actionId, var selectedRows)
    signal cellActivated(int row, int modelColumn)

    implicitWidth: 640
    implicitHeight: 360

    Component {
        id: defaultEditorComponent
        DataGridEditorDelegate {}
    }

    function defaultColumnWidth(logicalColumn) {
        if (logicalColumn >= 0 && logicalColumn < root.columnWidths.length)
            return Math.max(root.minimumColumnWidth, root.columnWidths[logicalColumn])
        return Math.max(root.minimumColumnWidth, 140)
    }

    function logicalColumnAtVisual(visualColumn) {
        if (visualColumn >= 0 && visualColumn < root._visualOrder.length)
            return root._visualOrder[visualColumn]
        return visualColumn
    }

    function visualColumnForLogical(logicalColumn) {
        var visual = root._visualOrder.indexOf(logicalColumn)
        return visual >= 0 ? visual : logicalColumn
    }

    function columnIdForModelColumn(logicalColumn) {
        if (root.columnState && root.columnState.idForModelColumn) {
            var id = root.columnState.idForModelColumn(logicalColumn)
            if (id && id.length > 0)
                return id
        }
        return "column-" + logicalColumn
    }

    function resolveCellDelegate(logicalColumn) {
        if (root.cellDelegateResolver) {
            var resolved = root.cellDelegateResolver(root.columnIdForModelColumn(logicalColumn), logicalColumn)
            if (resolved)
                return resolved
        }
        return root.cellDelegate
    }

    function resolveEditorDelegate(logicalColumn) {
        if (root.editorDelegateResolver) {
            var resolved = root.editorDelegateResolver(root.columnIdForModelColumn(logicalColumn), logicalColumn)
            if (resolved)
                return resolved
        }
        return defaultEditorComponent
    }

    function resolveEditorOptions(logicalColumn) {
        if (!root.editorOptionsResolver)
            return []
        var options = root.editorOptionsResolver(root.columnIdForModelColumn(logicalColumn), logicalColumn)
        return options === undefined || options === null ? [] : options
    }

    // Editor component contract: value, options, grid, row, modelColumn,
    // editorValue and focusEditor(). Built-in editor delegates implement all.
    function configureEditor(item, row, logicalColumn, value) {
        if (!item)
            return
        item.grid = root
        item.row = row
        item.modelColumn = logicalColumn
        item.options = root.resolveEditorOptions(logicalColumn)
        item.value = value
        if (item.syncValue)
            item.syncValue()
        if (item.focusEditor)
            Qt.callLater(item.focusEditor)
        else
            Qt.callLater(function() { item.forceActiveFocus() })
    }

    function editorValue(item, fallback) {
        return item && item.editorValue !== undefined ? item.editorValue : fallback
    }

    function validateEdit(row, modelColumn, value) {
        if (!root.editValidator)
            return { "accepted": true, "message": "" }
        var result = root.editValidator(row, modelColumn, value)
        if (result === undefined || result === true)
            return { "accepted": true, "message": "" }
        if (result === false)
            return { "accepted": false, "message": qsTr("Value rejected") }
        if (typeof result === "string")
            return { "accepted": false, "message": result }
        return {
            "accepted": result.accepted !== false,
            "message": result.message ? String(result.message) : ""
        }
    }

    function commitEdit(row, logicalColumn, value, directModelObject) {
        if (!root.editTransaction) {
            directModelObject.edit = value
            root.editCommitted(row, logicalColumn, value)
            return true
        }

        root.editTransaction.setTemporaryValue(value)
        var validation = root.validateEdit(row, logicalColumn, value)
        root.editTransaction.acceptValidation(validation.accepted, validation.message)
        if (validation.accepted && root.editTransaction.commit()) {
            root.editCommitted(row, logicalColumn, value)
            return true
        }

        var reason = validation.message.length > 0 ? validation.message : qsTr("Edit commit failed")
        root.editRejected(row, logicalColumn, value, reason)
        root.editTransaction.rollback()
        return false
    }

    function cancelActiveEdit() {
        if (root.editTransaction && root.editTransaction.active)
            root.editTransaction.rollback()
    }

    function selectedRows() { return selectionController.selectedRowsList() }
    function selectAllRows() { return selectionController.selectAllRows() }
    function clearSelection() { selectionController.clear() }

    function requestBatchAction(actionId) {
        var rows = root.selectedRows()
        if (rows.length === 0)
            return false
        root.batchActionRequested(actionId, rows)
        return true
    }

    function firstVisibleVisualColumn() {
        for (var visual = 0; visual < table.columns; ++visual) {
            var logical = root.logicalColumnAtVisual(visual)
            if (!root.columnState || root.columnState.isModelColumnVisible(logical))
                return visual
        }
        return -1
    }

    function lastVisibleVisualColumn() {
        for (var visual = table.columns - 1; visual >= 0; --visual) {
            var logical = root.logicalColumnAtVisual(visual)
            if (!root.columnState || root.columnState.isModelColumnVisible(logical))
                return visual
        }
        return -1
    }

    function nextVisibleVisualColumn(fromVisual, direction) {
        for (var visual = fromVisual + direction;
             visual >= 0 && visual < table.columns; visual += direction) {
            var logical = root.logicalColumnAtVisual(visual)
            if (!root.columnState || root.columnState.isModelColumnVisible(logical))
                return visual
        }
        return -1
    }

    function focusCell(row, logicalColumn) {
        if (row < 0 || row >= table.rows || logicalColumn < 0 || logicalColumn >= table.columns)
            return false
        var visual = root.visualColumnForLogical(logicalColumn)
        if (visual < 0 || visual >= table.columns)
            return false
        if (root.columnState && !root.columnState.isModelColumnVisible(logicalColumn))
            return false
        if (!selectionController.setCurrentCell(row, logicalColumn, true))
            return false
        table.forceActiveFocus()
        table.positionViewAtCell(Qt.point(visual, row), TableView.Contain)
        return true
    }

    function focusFirstCell() {
        if (table.rows <= 0)
            return false
        var visual = root.firstVisibleVisualColumn()
        return visual >= 0 && root.focusCell(0, root.logicalColumnAtVisual(visual))
    }

    function focusLastCell() {
        if (table.rows <= 0)
            return false
        var visual = root.lastVisibleVisualColumn()
        return visual >= 0 && root.focusCell(table.rows - 1, root.logicalColumnAtVisual(visual))
    }

    function advanceTab(forward) {
        if (table.rows <= 0 || table.columns <= 0)
            return false

        var row = selectionController.currentRow
        var logical = selectionController.currentColumn
        if (row < 0 || logical < 0)
            return forward ? root.focusFirstCell() : root.focusLastCell()

        var visual = root.visualColumnForLogical(logical)
        var nextVisual = root.nextVisibleVisualColumn(visual, forward ? 1 : -1)
        if (nextVisual >= 0)
            return root.focusCell(row, root.logicalColumnAtVisual(nextVisual))

        if (forward && row + 1 < table.rows) {
            var first = root.firstVisibleVisualColumn()
            return first >= 0 && root.focusCell(row + 1, root.logicalColumnAtVisual(first))
        }
        if (!forward && row > 0) {
            var last = root.lastVisibleVisualColumn()
            return last >= 0 && root.focusCell(row - 1, root.logicalColumnAtVisual(last))
        }

        if (root.crossGridNavigation) {
            var target = forward ? root.nextGrid : root.previousGrid
            if (target)
                return forward ? target.focusFirstCell() : target.focusLastCell()
        }
        return false
    }

    function handleNavigationKey(event) {
        if (event.key === Qt.Key_Tab)
            return root.advanceTab(true)
        if (event.key === Qt.Key_Backtab)
            return root.advanceTab(false)

        var row = selectionController.currentRow
        var logical = selectionController.currentColumn
        if (row < 0 || logical < 0)
            return false
        var visual = root.visualColumnForLogical(logical)

        if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
            var next = root.nextVisibleVisualColumn(visual, event.key === Qt.Key_Right ? 1 : -1)
            return next >= 0 && root.focusCell(row, root.logicalColumnAtVisual(next))
        }

        if (event.key === Qt.Key_Home || event.key === Qt.Key_End) {
            var edge = event.key === Qt.Key_Home
                     ? root.firstVisibleVisualColumn()
                     : root.lastVisibleVisualColumn()
            var targetRow = (event.modifiers & Qt.ControlModifier)
                          ? (event.key === Qt.Key_Home ? 0 : table.rows - 1)
                          : row
            return edge >= 0 && root.focusCell(targetRow, root.logicalColumnAtVisual(edge))
        }

        if (event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
            var pageRows = Math.max(1, Math.floor(table.height / Math.max(1, root.rowHeight)) - 1)
            var delta = event.key === Qt.Key_PageDown ? pageRows : -pageRows
            var target = Math.max(0, Math.min(table.rows - 1, row + delta))
            return root.focusCell(target, logical)
        }

        return false
    }

    function applyColumnState() {
        if (!root.columnState || table.columns <= 0)
            return

        root._restoringColumnState = true
        table.clearColumnReordering()
        table.clearColumnWidths()

        var current = []
        for (var i = 0; i < table.columns; ++i)
            current.push(i)

        for (var target = 0; target < root.columnState.count; ++target) {
            var logical = root.columnState.modelColumnAt(target)
            var from = current.indexOf(logical)
            if (from < 0 || from === target)
                continue
            table.moveColumn(from, target)
            current.splice(target, 0, current.splice(from, 1)[0])
        }

        root._visualOrder = current
        table.forceLayout()
        root._restoringColumnState = false
    }

    function captureExplicitWidths() {
        if (!root.columnState || root._restoringColumnState)
            return
        for (var visual = 0; visual < table.columns; ++visual) {
            var width = table.explicitColumnWidth(visual)
            if (width >= 0) {
                var logical = root.logicalColumnAtVisual(visual)
                root.columnState.setWidthByModelColumn(logical, width)
            }
        }
    }

    onColumnStateChanged: Qt.callLater(applyColumnState)
    onModelChanged: {
        if (root.editTransaction)
            root.editTransaction.model = root.model
    }
    onEditTransactionChanged: {
        if (root.editTransaction)
            root.editTransaction.model = root.model
    }

    ItemSelectionModel {
        id: selectionModel
        model: root.model
    }

    DataGridSelectionController {
        id: selectionController
        selectionModel: selectionModel
        selectRows: root.selectRows
    }

    TableView {
        id: table
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        model: root.model
        selectionModel: selectionModel
        selectionBehavior: root.selectRows ? TableView.SelectRows : TableView.SelectCells
        selectionMode: root.selectionMode
        keyNavigationEnabled: true
        pointerNavigationEnabled: true
        reuseItems: true
        alternatingRows: root.alternatingRows
        resizableColumns: root.resizableColumns
        resizableRows: false
        editTriggers: root.editable ? TableView.DoubleTapped | TableView.EditKeyPressed : TableView.NoEditTriggers
        columnSpacing: 1
        rowSpacing: 1
        boundsBehavior: Flickable.StopAtBounds
        rowHeightProvider: function(row) { return root.rowHeight }
        columnWidthProvider: function(visualColumn) {
            var explicitWidth = table.explicitColumnWidth(visualColumn)
            if (explicitWidth >= 0)
                return explicitWidth
            var logicalColumn = root.logicalColumnAtVisual(visualColumn)
            if (root.columnState) {
                if (!root.columnState.isModelColumnVisible(logicalColumn))
                    return 0
                return Math.max(root.minimumColumnWidth,
                                root.columnState.widthForModelColumn(logicalColumn,
                                                                    root.defaultColumnWidth(logicalColumn)))
            }
            return root.defaultColumnWidth(logicalColumn)
        }

        Keys.onPressed: function(event) {
            if (root.handleNavigationKey(event))
                event.accepted = true
        }

        onColumnMoved: function(logicalIndex, oldVisualIndex, newVisualIndex) {
            if (root._restoringColumnState)
                return
            var moved = root._visualOrder.splice(oldVisualIndex, 1)[0]
            root._visualOrder.splice(newVisualIndex, 0, moved)
            if (root.columnState)
                root.columnState.moveModelColumn(logicalIndex, newVisualIndex)
        }
        onLayoutChanged: root.captureExplicitWidths()

        delegate: Rectangle {
            id: cell
            required property bool selected
            required property bool current
            required property int row
            required property int column
            readonly property int modelColumn: root.logicalColumnAtVisual(column)

            implicitWidth: table.columnWidth(column)
            implicitHeight: root.rowHeight
            color: selected
                ? Tokens.accent
                : (root.alternatingRows && row % 2 ? Tokens.surfaceSecondary : Tokens.surface)
            border.color: current ? Tokens.accent : Tokens.border
            border.width: current ? 1 : 0

            Loader {
                id: cellLoader
                anchors.fill: parent
                sourceComponent: root.resolveCellDelegate(cell.modelColumn)
                active: sourceComponent !== null
                onLoaded: {
                    item.grid = root
                    item.row = cell.row
                    item.modelColumn = cell.modelColumn
                    item.value = model.display
                    item.selected = cell.selected
                    item.current = cell.current
                }
            }

            Binding { when: cellLoader.item !== null; target: cellLoader.item; property: "value"; value: model.display }
            Binding { when: cellLoader.item !== null; target: cellLoader.item; property: "selected"; value: cell.selected }
            Binding { when: cellLoader.item !== null; target: cellLoader.item; property: "current"; value: cell.current }
            Binding { when: cellLoader.item !== null; target: cellLoader.item; property: "row"; value: cell.row }
            Binding { when: cellLoader.item !== null; target: cellLoader.item; property: "modelColumn"; value: cell.modelColumn }

            Label {
                visible: cellLoader.sourceComponent === null
                anchors.fill: parent
                leftPadding: 10
                rightPadding: 10
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                color: selected ? Tokens.onScrim : Tokens.textPrimary
                text: model.display
            }

            TapHandler {
                enabled: root.contextMenuEnabled
                acceptedButtons: Qt.RightButton
                onTapped: function(eventPoint, button) {
                    if (!selectionController.isRowSelected(cell.row))
                        selectionController.selectRow(cell.row, false)
                    var point = cell.mapToItem(root, eventPoint.position.x, eventPoint.position.y)
                    root.contextMenuRequested(cell.row, cell.modelColumn,
                                              point.x, point.y, root.selectedRows())
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onDoubleTapped: function(eventPoint, button) {
                    root.cellActivated(cell.row, cell.modelColumn)
                }
            }

            TableView.editDelegate: FocusScope {
                id: editHost
                implicitWidth: cell.width
                implicitHeight: cell.height
                property var initialValue: model.edit === undefined ? model.display : model.edit

                Loader {
                    id: editorLoader
                    anchors.fill: parent
                    sourceComponent: root.resolveEditorDelegate(cell.modelColumn)
                    onLoaded: root.configureEditor(item, cell.row, cell.modelColumn, editHost.initialValue)
                }

                Component.onCompleted: {
                    if (root.editTransaction)
                        root.editTransaction.begin(cell.row, cell.modelColumn)
                }

                Component.onDestruction: {
                    if (root.editTransaction && root.editTransaction.active
                            && root.editTransaction.row === cell.row
                            && root.editTransaction.column === cell.modelColumn) {
                        root.editTransaction.rollback()
                    }
                }

                Keys.onEscapePressed: function(event) {
                    root.cancelActiveEdit()
                    table.closeEditor()
                    event.accepted = true
                }

                TableView.onCommit: {
                    var value = root.editorValue(editorLoader.item, editHost.initialValue)
                    root.commitEdit(cell.row, cell.modelColumn, value, model)
                }
            }
        }

        Component.onCompleted: {
            root._visualOrder = []
            for (var i = 0; i < table.columns; ++i)
                root._visualOrder.push(i)
            if (root.editTransaction)
                root.editTransaction.model = root.model
            Qt.callLater(root.applyColumnState)
        }
    }

    HorizontalHeaderView {
        id: header
        anchors.left: table.left
        anchors.right: table.right
        anchors.top: table.top
        height: root.headerHeight
        syncView: table
        movableColumns: root.movableColumns
        resizableColumns: root.resizableColumns
        clip: true
        z: 10

        delegate: Rectangle {
            implicitHeight: root.headerHeight
            color: Tokens.surfaceSecondary
            border.color: Tokens.border

            Label {
                anchors.fill: parent
                leftPadding: 10
                rightPadding: 10
                verticalAlignment: Text.AlignVCenter
                font.bold: true
                elide: Text.ElideRight
                color: Tokens.textPrimary
                text: display
            }
        }
    }
}
