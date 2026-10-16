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
    property Component cellDelegate: null

    readonly property alias tableView: table
    readonly property alias selectionModel: selectionModel
    property bool _restoringColumnState: false
    property var _visualOrder: []

    signal editCommitted(int row, int column, var value)
    signal editRejected(int row, int column, var value, string reason)

    implicitWidth: 640
    implicitHeight: 360

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

    function validateEdit(row, column, value) {
        if (!root.editValidator)
            return { "accepted": true, "message": "" }
        var result = root.editValidator(row, column, value)
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

            implicitWidth: table.columnWidth(column)
            implicitHeight: root.rowHeight
            color: selected
                ? Tokens.accent
                : (root.alternatingRows && row % 2 ? Tokens.surfaceSecondary : Tokens.surface)
            border.color: current ? Tokens.accent : Tokens.border
            border.width: current ? 1 : 0

            Loader {
                anchors.fill: parent
                sourceComponent: root.cellDelegate
                active: root.cellDelegate !== null
            }

            Label {
                visible: root.cellDelegate === null
                anchors.fill: parent
                leftPadding: 10
                rightPadding: 10
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                color: selected ? Tokens.onScrim : Tokens.textPrimary
                text: model.display
            }

            TableView.editDelegate: FocusScope {
                implicitWidth: cell.width
                implicitHeight: cell.height

                TextField {
                    id: editor
                    anchors.fill: parent
                    text: model.edit === undefined ? model.display : model.edit
                    selectByMouse: true

                    Component.onCompleted: {
                        forceActiveFocus()
                        selectAll()
                        if (root.editTransaction)
                            root.editTransaction.begin(cell.row, cell.column)
                    }
                }

                TableView.onCommit: {
                    if (!root.editTransaction) {
                        model.edit = editor.text
                        root.editCommitted(cell.row, cell.column, editor.text)
                        return
                    }

                    root.editTransaction.setTemporaryValue(editor.text)
                    var result = root.validateEdit(cell.row, cell.column, editor.text)
                    root.editTransaction.acceptValidation(result.accepted, result.message)
                    if (result.accepted && root.editTransaction.commit()) {
                        root.editCommitted(cell.row, cell.column, editor.text)
                    } else {
                        var reason = result.message.length > 0 ? result.message : qsTr("Edit commit failed")
                        root.editRejected(cell.row, cell.column, editor.text, reason)
                        root.editTransaction.rollback()
                    }
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
