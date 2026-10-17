import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

// Search/filter a ComboBox's display texts without copying or mutating its model.
LfComboBox {
    id: root

    property string searchText: ""
    property int maxPopupHeight: 340
    readonly property var filteredIndices: {
        const indices = []
        const query = searchText.trim().toLocaleLowerCase()
        for (let i = 0; i < count; ++i) {
            if (String(textAt(i)).toLocaleLowerCase().indexOf(query) !== -1)
                indices.push(i)
        }
        return indices
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
            searchField.forceActiveFocus()
        }

        contentItem: ColumnLayout {
            spacing: Units.xs

            LfSearchBox {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("Filter options")
                onTextChanged: root.searchText = text
            }

            ListView {
                id: optionsView
                Layout.fillWidth: true
                Layout.preferredHeight: picker.listHeight
                clip: true
                model: root.filteredIndices
                currentIndex: -1
                boundsBehavior: Flickable.StopAtBounds

                delegate: Controls.ItemDelegate {
                    required property int modelData
                    readonly property int sourceIndex: modelData
                    width: ListView.view ? ListView.view.width : root.width
                    implicitHeight: Tokens.controlHeight
                    text: root.textAt(sourceIndex)
                    highlighted: root.currentIndex === sourceIndex
                    onClicked: {
                        root.currentIndex = sourceIndex
                        root.activated(sourceIndex)
                        picker.close()
                    }
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
