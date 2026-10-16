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
    readonly property var editorValue: editor.currentText

    implicitWidth: 120
    implicitHeight: Tokens.controlHeight

    function syncValue() {
        if (!editor || !root.options)
            return
        var index = root.options.indexOf(root.value)
        if (index < 0)
            index = root.options.indexOf(String(root.value))
        editor.currentIndex = index >= 0 ? index : (root.options.length > 0 ? 0 : -1)
    }

    function focusEditor() {
        editor.forceActiveFocus()
        editor.popup.open()
    }

    onValueChanged: syncValue()
    onOptionsChanged: syncValue()

    ComboBox {
        id: editor
        anchors.fill: parent
        model: root.options
        Component.onCompleted: root.syncValue()
    }
}
