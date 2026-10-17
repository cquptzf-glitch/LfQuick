import QtQuick
import QtQuick.Controls as Controls
import QtQml.Models

Controls.Menu {
    id: root

    property var model: []

    signal itemTriggered(int index, var value)

    function valueAt(index) {
        if (!model)
            return undefined
        if (typeof model.get === "function")
            return model.get(index)
        return model[index]
    }

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
            readonly property var value: root.valueAt(index)

            text: root.textFor(value)
            enabled: root.enabledFor(value)
            checkable: root.checkableFor(value)
            checked: root.checkedFor(value)

            onTriggered: root.itemTriggered(index, value)
        }

        onObjectAdded: function(index, object) {
            root.insertItem(index, object)
        }

        onObjectRemoved: function(index, object) {
            root.removeItem(object)
        }
    }
}
