import QtQuick
import QtQuick.Controls
import LfQuick

Item {
    id: root

    property var value: false
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null

    implicitWidth: 52
    implicitHeight: Tokens.controlHeight

    function checkedValue() {
        if (typeof value === "boolean")
            return value
        if (typeof value === "number")
            return value !== 0
        var text = String(value).toLowerCase()
        return text === "true" || text === "1" || text === "yes" || text === "checked"
    }

    CheckBox {
        anchors.centerIn: parent
        checked: root.checkedValue()
        enabled: false
    }
}
