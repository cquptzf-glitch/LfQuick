import QtQuick
import QtQuick.Controls
import LfQuick

FocusScope {
    id: root

    property var value: ""
    property var options: []
    property var grid: null
    property int row: -1
    property int modelColumn: -1
    property alias editorValue: editor.text

    implicitWidth: editor.implicitWidth
    implicitHeight: editor.implicitHeight

    function focusEditor() {
        editor.forceActiveFocus()
        editor.selectAll()
    }

    TextField {
        id: editor
        anchors.fill: parent
        text: root.value === undefined || root.value === null ? "" : String(root.value)
        selectByMouse: true
    }
}
