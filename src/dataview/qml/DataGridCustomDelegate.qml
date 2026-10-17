import QtQuick
import LfQuick

Item {
    id: root

    property var value: null
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null
    property Component contentComponent
    property var context: ({})

    implicitWidth: loader.item ? loader.item.implicitWidth : 120
    implicitHeight: loader.item ? loader.item.implicitHeight : Tokens.controlHeight

    function syncItem() {
        if (!loader.item)
            return
        if (loader.item.value !== undefined)
            loader.item.value = root.value
        if (loader.item.selected !== undefined)
            loader.item.selected = root.selected
        if (loader.item.current !== undefined)
            loader.item.current = root.current
        if (loader.item.row !== undefined)
            loader.item.row = root.row
        if (loader.item.modelColumn !== undefined)
            loader.item.modelColumn = root.modelColumn
        if (loader.item.grid !== undefined)
            loader.item.grid = root.grid
        if (loader.item.context !== undefined)
            loader.item.context = root.context
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: root.contentComponent
        onLoaded: root.syncItem()
    }

    onValueChanged: syncItem()
    onSelectedChanged: syncItem()
    onCurrentChanged: syncItem()
    onRowChanged: syncItem()
    onModelColumnChanged: syncItem()
    onGridChanged: syncItem()
    onContextChanged: syncItem()
}
