import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

// Search/filter a ComboBox's display texts without copying or mutating its model.
LfComboBox {
    id: root

    property string searchText: ""
    property int maxPopupHeight: 340
    property bool _componentReady: false
    property int highlightedFilteredIndex: -1

    Component.onCompleted: {
        _componentReady = true
        resetHighlight()
    }
    readonly property var filteredIndices: {
        if (!_componentReady)
            return []
        const indices = []
        const query = searchText.trim().toLocaleLowerCase()
        for (let i = 0; i < count; ++i) {
            if (String(textAt(i)).toLocaleLowerCase().indexOf(query) !== -1)
                indices.push(i)
        }
        return indices
    }

    onSearchTextChanged: resetHighlight()

    function resetHighlight() {
        const selected = filteredIndices.indexOf(currentIndex)
        highlightedFilteredIndex = selected >= 0 ? selected
                                 : (filteredIndices.length > 0 ? 0 : -1)
    }

    // Returns the original source-model index (not a filtered visual row).
    function moveHighlight(direction) {
        const count = filteredIndices.length
        if (count === 0) {
            highlightedFilteredIndex = -1
            return -1
        }
        let next = highlightedFilteredIndex
        if (next < 0)
            next = direction < 0 ? count - 1 : 0
        else
            next = Math.max(0, Math.min(count - 1, next + direction))
        highlightedFilteredIndex = next
        if (picker.visible && optionsView.count > next)
            optionsView.positionViewAtIndex(next, ListView.Contain)
        return filteredIndices[next]
    }

    function activateFiltered(index) {
        if (!enabled || index < 0 || index >= filteredIndices.length)
            return false
        const sourceIndex = filteredIndices[index]
        currentIndex = sourceIndex
        activated(sourceIndex)
        picker.close()
        return true
    }

    popup: Controls.Popup {
        id: picker
        y: root.height + 2
        width: root.width
        padding: Units.xs
        focus: true
        implicitHeight: Math.min(root.maxPopupHeight,
                                 searchField.implicitHeight + listHeight + Units.sm + topPadding + bottomPadding)
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent

        readonly property real listHeight: Math.min(250,
            Math.max(Tokens.controlHeight, root.filteredIndices.length * Tokens.controlHeight))

        onOpened: {
            root.searchText = ""
            searchField.text = ""
            root.resetHighlight()
            searchField.forceActiveFocus()
        }

        contentItem: ColumnLayout {
            spacing: Units.xs

            LfSearchBox {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("Filter options")
                onTextChanged: root.searchText = text
                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Down) {
                        root.moveHighlight(1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        root.moveHighlight(-1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root.activateFiltered(root.highlightedFilteredIndex)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape) {
                        picker.close()
                        event.accepted = true
                    }
                }
            }

            ListView {
                id: optionsView
                Layout.fillWidth: true
                Layout.preferredHeight: picker.listHeight
                clip: true
                model: root.filteredIndices
                currentIndex: root.highlightedFilteredIndex
                boundsBehavior: Flickable.StopAtBounds

                delegate: Controls.ItemDelegate {
                    required property int modelData
                    readonly property int sourceIndex: modelData
                    width: ListView.view ? ListView.view.width : root.width
                    implicitHeight: Tokens.controlHeight
                    text: root.textAt(sourceIndex)
                    highlighted: index === root.highlightedFilteredIndex
                    hoverEnabled: true
                    onHoveredChanged: {
                        if (hovered)
                            root.highlightedFilteredIndex = index
                    }
                    onClicked: root.activateFiltered(index)
                    background: Rectangle {
                        radius: Tokens.radiusSmall
                        color: parent.hovered || parent.highlighted
                               ? Tokens.controlFillHover : "transparent"
                    }
                }
                Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
            }

            Text {
                Layout.fillWidth: true
                visible: root.filteredIndices.length === 0
                text: qsTr("No matching items")
                color: Tokens.textSecondary
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Typography.caption
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
