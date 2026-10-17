import QtQuick
import QtQuick.Controls
import LfQuick

HorizontalHeaderView {
    id: root

    property var grid: null
    property int headerHeight: grid && grid.headerHeight !== undefined
                               ? grid.headerHeight
                               : Units.controlHeight
    property bool showBorders: true

    syncView: grid && grid.tableView !== undefined ? grid.tableView : null
    height: headerHeight
    movableColumns: grid && grid.movableColumns !== undefined ? grid.movableColumns : true
    resizableColumns: grid && grid.resizableColumns !== undefined ? grid.resizableColumns : true
    clip: true

    delegate: Rectangle {
        implicitHeight: root.headerHeight
        color: Tokens.surfaceSecondary
        border.color: root.showBorders ? Tokens.border : "transparent"

        Label {
            anchors.fill: parent
            leftPadding: Units.controlPaddingHorizontal
            rightPadding: Units.controlPaddingHorizontal
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Typography.body
            font.weight: Typography.weightSemibold
            elide: Text.ElideRight
            color: Tokens.textPrimary
            text: display
        }
    }
}
