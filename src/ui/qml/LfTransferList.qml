import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

// Transfers stable keys between available/assigned lists.
// Intended for bounded JS arrays / QML ListModels, not virtualized datasets.
Controls.Control {
    id: root

    property var model: []
    property string textRole: "text"
    property string valueRole: "value"
    property string availableTitle: qsTr("Available")
    property string selectedTitle: qsTr("Selected")
    property var targetValues: []
    property var leftSelectedValues: []
    property var rightSelectedValues: []

    signal itemsTransferred(var values)

    readonly property int optionCount: model
        ? (typeof model.count === "number" ? model.count
           : (typeof model.length === "number" ? model.length : 0))
        : 0
    readonly property var availableIndices: {
        const indices = []
        for (let i = 0; i < optionCount; ++i) {
            if (targetValues.indexOf(keyAt(i)) === -1)
                indices.push(i)
        }
        return indices
    }
    readonly property var assignedIndices: {
        const indices = []
        for (let i = 0; i < targetValues.length; ++i) {
            const index = indexForKey(targetValues[i])
            if (index >= 0)
                indices.push(index)
        }
        return indices
    }

    function valueAt(index) {
        if (index < 0 || index >= optionCount)
            return undefined
        return model && typeof model.get === "function" ? model.get(index) : model[index]
    }
    function keyAt(index) {
        const value = valueAt(index)
        if (value === undefined)
            return undefined
        if (value !== null && typeof value === "object") {
            if (value[valueRole] !== undefined)
                return value[valueRole]
            if (value.id !== undefined)
                return value.id
        }
        return value
    }
    function textAt(index) {
        const value = valueAt(index)
        if (value === undefined || value === null)
            return ""
        if (typeof value !== "object")
            return String(value)
        if (value[textRole] !== undefined)
            return String(value[textRole])
        if (value.label !== undefined)
            return String(value.label)
        return String(keyAt(index))
    }
    function indexForKey(key) {
        for (let i = 0; i < optionCount; ++i) {
            if (keyAt(i) === key)
                return i
        }
        return -1
    }
    function toggleLeft(key) {
        const next = leftSelectedValues.slice()
        const index = next.indexOf(key)
        if (index >= 0)
            next.splice(index, 1)
        else
            next.push(key)
        leftSelectedValues = next
    }
    function toggleRight(key) {
        const next = rightSelectedValues.slice()
        const index = next.indexOf(key)
        if (index >= 0)
            next.splice(index, 1)
        else
            next.push(key)
        rightSelectedValues = next
    }
    function commit(next) {
        if (next.length === targetValues.length
                && next.every(function(key, index) { return key === targetValues[index] }))
            return false
        targetValues = next
        leftSelectedValues = []
        rightSelectedValues = []
        itemsTransferred(next.slice())
        return true
    }
    function addIndex(index) {
        if (index < 0 || index >= optionCount)
            return false
        const value = valueAt(index)
        if (value && typeof value === "object" && value.enabled === false)
            return false
        const key = keyAt(index)
        if (targetValues.indexOf(key) !== -1)
            return false
        return commit(targetValues.concat([key]))
    }
    function removeKey(key) {
        const next = targetValues.filter(function(item) { return item !== key })
        return commit(next)
    }
    function moveSelectedToTarget() {
        let next = targetValues.slice()
        for (const key of leftSelectedValues) {
            const index = indexForKey(key)
            const item = valueAt(index)
            if (index >= 0 && next.indexOf(key) === -1
                    && !(item && typeof item === "object" && item.enabled === false))
                next.push(key)
        }
        return commit(next)
    }
    function moveSelectedToSource() {
        const next = targetValues.filter(function(key) {
            return rightSelectedValues.indexOf(key) === -1
        })
        return commit(next)
    }
    function moveAllToTarget() {
        const next = targetValues.slice()
        for (let i = 0; i < optionCount; ++i) {
            const item = valueAt(i)
            const key = keyAt(i)
            if (next.indexOf(key) === -1
                    && !(item && typeof item === "object" && item.enabled === false))
                next.push(key)
        }
        return commit(next)
    }
    function moveAllToSource() {
        return commit([])
    }

    implicitWidth: 580
    implicitHeight: 300
    padding: Units.sm

    contentItem: RowLayout {
        spacing: Units.md

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Units.xs
            Text {
                text: root.availableTitle
                color: Tokens.textPrimary
                font.pixelSize: Typography.body
                font.weight: Typography.weightSemibold
            }
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: root.availableIndices
                boundsBehavior: Flickable.StopAtBounds
                delegate: LfListItem {
                    required property int modelData
                    readonly property int sourceIndex: modelData
                    width: ListView.view ? ListView.view.width : 200
                    title: root.textAt(sourceIndex)
                    selected: root.leftSelectedValues.indexOf(root.keyAt(sourceIndex)) !== -1
                    compact: true
                    enabled: {
                        const item = root.valueAt(sourceIndex)
                        return !(item && typeof item === "object" && item.enabled === false)
                    }
                    onClicked: root.toggleLeft(root.keyAt(sourceIndex))
                }
                Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: Units.sm
            LfToolButton {
                text: "›"
                Accessible.name: qsTr("Add selected")
                enabled: root.leftSelectedValues.length > 0
                onClicked: root.moveSelectedToTarget()
            }
            LfToolButton {
                text: "»"
                Accessible.name: qsTr("Add all")
                onClicked: root.moveAllToTarget()
            }
            LfToolButton {
                text: "‹"
                Accessible.name: qsTr("Remove selected")
                enabled: root.rightSelectedValues.length > 0
                onClicked: root.moveSelectedToSource()
            }
            LfToolButton {
                text: "«"
                Accessible.name: qsTr("Remove all")
                enabled: root.targetValues.length > 0
                onClicked: root.moveAllToSource()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Units.xs
            Text {
                text: root.selectedTitle
                color: Tokens.textPrimary
                font.pixelSize: Typography.body
                font.weight: Typography.weightSemibold
            }
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: root.assignedIndices
                boundsBehavior: Flickable.StopAtBounds
                delegate: LfListItem {
                    required property int modelData
                    readonly property int sourceIndex: modelData
                    width: ListView.view ? ListView.view.width : 200
                    title: root.textAt(sourceIndex)
                    selected: root.rightSelectedValues.indexOf(root.keyAt(sourceIndex)) !== -1
                    compact: true
                    onClicked: root.toggleRight(root.keyAt(sourceIndex))
                }
                Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
            }
        }
    }
    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Tokens.surface
        border.width: 1
        border.color: Tokens.border
    }
}
