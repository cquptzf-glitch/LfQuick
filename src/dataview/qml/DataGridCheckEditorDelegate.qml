import QtQuick
import QtQuick.Controls
import LfQuick

FocusScope {
    id: root

    property var value: false
    property var options: []
    property var grid: null
    property int row: -1
    property int modelColumn: -1
    readonly property var editorValue: editor.checked

    implicitWidth: 52
    implicitHeight: Tokens.controlHeight

    function normalizedValue() {
        if (typeof value === "boolean")
            return value
        if (typeof value === "number")
            return value !== 0
        var text = String(value).toLowerCase()
        return text === "true" || text === "1" || text === "yes" || text === "checked"
    }

    function syncValue() {
        editor.checked = normalizedValue()
    }

    function focusEditor() {
        editor.forceActiveFocus()
    }

    onValueChanged: syncValue()

    CheckBox {
        id: editor
        anchors.centerIn: parent
        Component.onCompleted: root.syncValue()
    }
}
