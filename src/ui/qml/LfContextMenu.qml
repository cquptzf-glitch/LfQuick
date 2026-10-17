import QtQuick
import QtQuick.Controls as Controls
import QtQml.Models

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
        const px = xPosition === undefined ? 0 : xPosition
        const py = yPosition === undefined ? 0 : yPosition
        if (item)
            root.popup(item, px, py)
        else
            root.popup(px, py)
    }

    Instantiator {
        id: itemInstantiator
        model: root.model

        delegate: Controls.MenuItem {
            required property int index
            required property var modelData

            text: root.textFor(modelData)
            enabled: root.enabledFor(modelData)
            checkable: root.checkableFor(modelData)
            checked: root.checkedFor(modelData)

            onTriggered: root.itemTriggered(index, modelData)
        }

        onObjectAdded: function(index, object) {
            root.insertItem(index, object)
        }

        onObjectRemoved: function(index, object) {
            root.removeItem(object)
        }
    }
}
