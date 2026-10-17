import QtQuick

QtObject {
    id: root

    property var grid
    property bool enabled: true
    property bool selectAllEnabled: true
    property bool enterActivatesCell: true
    property bool escapeCancelsEdit: true

    function handle(event) {
        if (!enabled || !grid || !event)
            return false

        if (selectAllEnabled
                && event.key === Qt.Key_A
                && (event.modifiers & Qt.ControlModifier)) {
            grid.selectAllRows()
            return true
        }

        if (event.key === Qt.Key_Tab)
            return grid.advanceTab(true)
        if (event.key === Qt.Key_Backtab)
            return grid.advanceTab(false)

        const row = grid.selectionController.currentRow
        const logical = grid.selectionController.currentColumn
        if (row < 0 || logical < 0)
            return false

        if (event.key === Qt.Key_Up)
            return row > 0 && grid.focusCell(row - 1, logical)
        if (event.key === Qt.Key_Down)
            return row + 1 < grid.tableView.rows && grid.focusCell(row + 1, logical)

        const visual = grid.visualColumnForLogical(logical)
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
            const next = grid.nextVisibleVisualColumn(
                visual,
                event.key === Qt.Key_Right ? 1 : -1)
            return next >= 0 && grid.focusCell(row, grid.logicalColumnAtVisual(next))
        }

        if (event.key === Qt.Key_Home || event.key === Qt.Key_End) {
            const edge = event.key === Qt.Key_Home
                       ? grid.firstVisibleVisualColumn()
                       : grid.lastVisibleVisualColumn()
            const targetRow = (event.modifiers & Qt.ControlModifier)
                            ? (event.key === Qt.Key_Home ? 0 : grid.tableView.rows - 1)
                            : row
            return edge >= 0 && grid.focusCell(targetRow, grid.logicalColumnAtVisual(edge))
        }

        if (event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
            const pageRows = Math.max(
                1,
                Math.floor(grid.tableView.height / Math.max(1, grid.rowHeight)) - 1)
            const delta = event.key === Qt.Key_PageDown ? pageRows : -pageRows
            const target = Math.max(0, Math.min(grid.tableView.rows - 1, row + delta))
            return grid.focusCell(target, logical)
        }

        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                && enterActivatesCell) {
            grid.cellActivated(row, logical)
            return true
        }

        if (event.key === Qt.Key_Escape && escapeCancelsEdit) {
            grid.cancelActiveEdit()
            if (grid.tableView && grid.tableView.closeEditor)
                grid.tableView.closeEditor()
            return true
        }

        return false
    }
}
