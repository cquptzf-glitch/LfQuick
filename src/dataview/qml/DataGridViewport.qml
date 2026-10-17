import QtQuick
import QtQuick.Controls
import LfQuick

TableView {
    id: root

    property int rowHeight: Units.controlHeight
    property int minimumColumnWidth: 80
    property real defaultColumnWidth: 140
    property var columnWidthResolver: null
    property bool showGridLines: true

    keyNavigationEnabled: true
    pointerNavigationEnabled: true
    reuseItems: true
    resizableColumns: true
    resizableRows: false
    boundsBehavior: Flickable.StopAtBounds
    columnSpacing: showGridLines ? 1 : 0
    rowSpacing: showGridLines ? 1 : 0

    rowHeightProvider: function(row) {
        return root.rowHeight
    }

    columnWidthProvider: function(column) {
        if (root.columnWidthResolver) {
            var width = root.columnWidthResolver(column)
            if (width !== undefined && width !== null && width >= 0)
                return Math.max(root.minimumColumnWidth, width)
        }
        var explicitWidth = root.explicitColumnWidth(column)
        if (explicitWidth >= 0)
            return explicitWidth
        return Math.max(root.minimumColumnWidth, root.defaultColumnWidth)
    }
}
