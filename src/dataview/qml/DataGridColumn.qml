import QtQuick

QtObject {
    id: root

    property string columnId: ""
    property int modelColumn: -1
    property string title: columnId
    property real width: 140
    property real minimumWidth: 80
    property real maximumWidth: 100000
    property bool visible: true
    property bool sortable: true
    property bool filterable: true
    property bool editable: true
    property int sortOrder: Qt.AscendingOrder
    property var filterValue: undefined
    property Component cellDelegate: null
    property Component editorDelegate: null
    property var editorOptions: []
    property var metadata: ({})

    function effectiveWidth() {
        return Math.max(minimumWidth, Math.min(maximumWidth, width))
    }

    function state() {
        return {
            "id": columnId,
            "modelColumn": modelColumn,
            "title": title,
            "width": effectiveWidth(),
            "visible": visible,
            "sortOrder": sortOrder,
            "filter": filterValue
        }
    }
}
