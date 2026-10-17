import QtQuick
import QtQuick.Controls as Controls

// JS array / QML ListModel multi-select. Use stable valueRole keys for objects.
Controls.Control {
    id: root

    property var model: []
    property string textRole: "text"
    property string valueRole: "value"
    property var selectedValues: []
    property string placeholderText: qsTr("Select items")
    property int maximumSelection: 0 // 0 = unlimited
    property int popupMaxHeight: 300

    signal selectionCommitted(var values)
    signal optionToggled(int index, var key, bool selected)

    readonly property int optionCount: model
        ? (typeof model.count === "number" ? model.count
           : (typeof model.length === "number" ? model.length : 0))
        : 0
    readonly property string summaryText: {
        const labels = []
        for (let i = 0; i < root.optionCount; ++i) {
            if (root.isSelected(i))
                labels.push(root.textAt(i))
        }
        return labels.length > 0 ? labels.join(", ") : root.placeholderText
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
    function isSelected(index) {
        return selectedValues.indexOf(keyAt(index)) !== -1
    }
    function toggleIndex(index) {
        if (index < 0 || index >= optionCount)
            return false
        const option = valueAt(index)
        if (option && typeof option === "object" && option.enabled === false)
            return false
        const key = keyAt(index)
        const next = selectedValues.slice()
        const selectedIndex = next.indexOf(key)
        if (selectedIndex >= 0)
            next.splice(selectedIndex, 1)
        else {
            if (maximumSelection > 0 && next.length >= maximumSelection)
                return false
            next.push(key)
        }
        selectedValues = next
        optionToggled(index, key, selectedIndex < 0)
        selectionCommitted(next.slice())
        return true
    }
    function clearSelection() {
        if (selectedValues.length === 0)
            return
        selectedValues = []
        selectionCommitted([])
    }
    function open() {
        if (enabled)
            optionsPopup.open()
    }
    function close() {
        optionsPopup.close()
    }

    implicitWidth: 240
    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: 0
    bottomPadding: 0
    activeFocusOnTab: true

    Accessible.role: Accessible.ComboBox
    Accessible.name: summaryText

    Keys.onSpacePressed: root.open()
    Keys.onReturnPressed: root.open()

    contentItem: Row {
        spacing: Units.sm
        Text {
            width: Math.max(0, parent.width - indicator.implicitWidth - parent.spacing)
            anchors.verticalCenter: parent.verticalCenter
            text: root.summaryText
            color: root.selectedValues.length ? Tokens.textPrimary : Tokens.textSecondary
            font.pixelSize: Typography.body
            elide: Text.ElideRight
        }
        Text {
            id: indicator
            anchors.verticalCenter: parent.verticalCenter
            text: "⌄"
            color: Tokens.textSecondary
        }
        TapHandler { onTapped: root.open() }
    }
    background: Rectangle {
        radius: Tokens.radiusSmall
        color: root.enabled ? Tokens.controlFill : Tokens.controlFillDisabled
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? Tokens.focusRing : Tokens.border
    }

    Controls.Popup {
        id: optionsPopup
        parent: root
        x: 0
        y: root.height + 2
        width: root.width
        height: Math.min(root.popupMaxHeight,
                         Math.max(Tokens.controlHeight, root.optionCount * Tokens.controlHeight + 2 * Units.xs))
        padding: Units.xs
        focus: true
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

        contentItem: ListView {
            clip: true
            model: root.optionCount
            boundsBehavior: Flickable.StopAtBounds
            delegate: Controls.CheckDelegate {
                required property int index
                width: ListView.view ? ListView.view.width : optionsPopup.width
                implicitHeight: Tokens.controlHeight
                text: root.textAt(index)
                // Prevent AbstractButton from replacing the checked binding.
                checkable: false
                checked: root.isSelected(index)
                enabled: {
                    const value = root.valueAt(index)
                    return !(value && typeof value === "object" && value.enabled === false)
                }
                onClicked: root.toggleIndex(index)
            }
            Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
        }
        background: Rectangle {
            radius: Tokens.radiusMedium
            color: Tokens.surface
            border.width: 1
            border.color: Tokens.border
        }
    }
}
