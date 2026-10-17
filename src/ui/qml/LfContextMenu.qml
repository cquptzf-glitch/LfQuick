import QtQuick
import QtQuick.Controls as Controls

Controls.Menu {
    id: root

    property var model: []

    signal itemTriggered(int index, var value)

    function textFor(value) {
        if (typeof value === "string")
            return value
        return value && value.text !== undefined ? String(value.text) : String(value)
    }

    function enabledFor(value) {
        return !(value && typeof value === "object" && value.enabled === false)
    }

    function checkableFor(value) {
        return value && typeof value === "object" && value.checkable === true
    }

    function checkedFor(value) {
        return value && typeof value === "object" && value.checked === true
    }

    function popupAt(item, xPosition, yPosition) {
        if (item)
            root.parent = item
        root.x = xPosition === undefined ? 0 : xPosition
        root.y = yPosition === undefined ? 0 : yPosition
        root.open()
    }

    Repeater {
        model: root.model

        Controls.MenuItem {
            required property int index
            required property var modelData

            text: root.textFor(modelData)
            enabled: root.enabledFor(modelData)
            checkable: root.checkableFor(modelData)
            checked: root.checkedFor(modelData)

            onTriggered: root.itemTriggered(index, modelData)
        }
    }
}
