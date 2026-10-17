import QtQuick
import QtQuick.Controls as Controls
import LfQuick

// QModelIndex-valued picker; model must derive from QAbstractItemModel.
Controls.Control {
    id: root

    property var model: null
    property var selectedIndex: undefined
    property string selectedText: ""
    property string placeholderText: qsTr("Select an item")
    property int popupHeight: 300

    signal itemSelected(var modelIndex, string label)

    function choose(modelIndex, label) {
        selectedIndex = modelIndex
        selectedText = String(label)
        itemSelected(modelIndex, selectedText)
        picker.close()
    }
    function open() {
        if (enabled && model)
            picker.open()
    }
    function close() {
        picker.close()
    }

    implicitWidth: 240
    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    padding: 0
    activeFocusOnTab: true

    Accessible.role: Accessible.ComboBox
    Accessible.name: selectedText.length > 0 ? selectedText : placeholderText

    Keys.onSpacePressed: root.open()
    Keys.onReturnPressed: root.open()

    contentItem: Row {
        spacing: Units.sm
        Text {
            width: Math.max(0, parent.width - arrow.implicitWidth - parent.spacing)
            anchors.verticalCenter: parent.verticalCenter
            text: root.selectedText.length > 0 ? root.selectedText : root.placeholderText
            color: root.selectedText.length > 0 ? Tokens.textPrimary : Tokens.textSecondary
            font.pixelSize: Typography.body
            elide: Text.ElideRight
        }
        Text {
            id: arrow
            anchors.verticalCenter: parent.verticalCenter
            text: "⌄"
            color: Tokens.textSecondary
            font.pixelSize: Typography.body
        }
        TapHandler { onTapped: root.open() }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: !root.enabled ? Tokens.controlFillDisabled : Tokens.controlFill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? Tokens.focusRing : Tokens.border
    }

    Controls.Popup {
        id: picker
        parent: root
        x: 0
        y: root.height + 2
        width: Math.max(root.width, 240)
        height: root.popupHeight
        padding: Units.xs
        focus: true
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

        contentItem: LfTreeView {
            id: tree
            model: root.model
            onNodeActivated: function(modelIndex, label) {
                root.choose(modelIndex, label)
            }
        }
        background: Rectangle {
            radius: Tokens.radiusMedium
            color: Tokens.surface
            border.width: 1
            border.color: Tokens.border
        }
    }
}
