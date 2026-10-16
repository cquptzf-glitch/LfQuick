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
    property bool movableColumns: true
    property bool resizableColumns: true
    property bool alternatingRows: true
    property bool selectRows: true
    property int selectionMode: TableView.ExtendedSelection
    property Component cellDelegate: null

    readonly property alias tableView: table
    readonly property alias selectionModel: selectionModel

    implicitWidth: 640
    implicitHeight: 360

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
        columnSpacing: 1
        rowSpacing: 1
        boundsBehavior: Flickable.StopAtBounds
        rowHeightProvider: function(row) { return root.rowHeight }
        columnWidthProvider: function(column) {
            if (column >= 0 && column < root.columnWidths.length)
                return Math.max(root.minimumColumnWidth, root.columnWidths[column])
            return Math.max(root.minimumColumnWidth, 140)
        }

        delegate: Rectangle {
            required property bool selected
            required property bool current
            required property int row
            required property int column

            implicitWidth: root.columnWidthProvider(column)
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
