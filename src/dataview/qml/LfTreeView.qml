import QtQuick
import QtQuick.Controls as Controls
import QtQml.Models
import LfQuick

// A view-only adapter for a hierarchical QAbstractItemModel.
// Visual rows are NOT source-model rows. Public events carry QModelIndex.
TreeView {
    id: root

    property int rowHeight: Tokens.controlHeight + Units.xs
    property bool activateOnSingleClick: true
    property bool alternatingRows: true

    signal nodeActivated(var modelIndex, string label)

    implicitWidth: 300
    implicitHeight: 280
    clip: true
    reuseItems: true
    selectionBehavior: TableView.SelectRows
    selectionMode: TableView.SingleSelection
    editTriggers: TableView.NoEditTriggers

    selectionModel: ItemSelectionModel {
        model: root.model
    }

    function activateRow(viewRow, label) {
        if (viewRow < 0 || viewRow >= rows || !selectionModel)
            return false
        const sourceIndex = index(viewRow, 0)
        selectionModel.setCurrentIndex(sourceIndex, ItemSelectionModel.ClearAndSelect)
        nodeActivated(sourceIndex, label === undefined ? "" : String(label))
        return true
    }

    delegate: Controls.TreeViewDelegate {
        id: cell

        required property int row
        required property int column
        required property var model
        text: model && model.display !== undefined ? String(model.display) : ""

        implicitWidth: Math.max(root.width, 240)
        implicitHeight: root.rowHeight
        indentation: Units.iconSize
        leftMargin: Units.sm
        rightMargin: Units.sm
        hoverEnabled: true

        contentItem: Text {
            text: cell.text
            color: cell.enabled ? Tokens.textPrimary : Tokens.textDisabled
            font: cell.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: cell.selected ? Tokens.selection
                 : cell.down ? Tokens.controlFillPressed
                 : cell.hovered ? Tokens.controlFillHover
                 : (root.alternatingRows && cell.row % 2 === 1 ? Tokens.surfaceSecondary : Tokens.surface)
            border.width: cell.activeFocus ? 2 : 0
            border.color: Tokens.focusRing
        }

        onClicked: {
            if (root.activateOnSingleClick)
                root.activateRow(row, text)
        }
        onDoubleClicked: {
            if (!root.activateOnSingleClick)
                root.activateRow(row, text)
        }
    }
}
