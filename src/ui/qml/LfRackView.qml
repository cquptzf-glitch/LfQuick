import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property var model: []
    property int columns: 8
    property int slotWidth: 58
    property int slotHeight: 58
    property int rowSpacing: Tokens.spacingSmall
    property int columnSpacing: Tokens.spacingSmall
    property Component slotDelegate

    signal slotClicked(int index, var entry)

    implicitWidth: grid.implicitWidth + leftPadding + rightPadding
    implicitHeight: grid.implicitHeight + topPadding + bottomPadding
    padding: Tokens.spacingMedium

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Tokens.surfaceSecondary
        border.color: Tokens.border
    }

    contentItem: GridLayout {
        id: grid
        columns: Math.max(1, root.columns)
        rowSpacing: root.rowSpacing
        columnSpacing: root.columnSpacing

        Repeater {
            model: root.model

            delegate: Loader {
                id: slotLoader
                required property int index
                required property var modelData

                property var entry: modelData
                Layout.preferredWidth: root.slotWidth
                Layout.preferredHeight: root.slotHeight
                sourceComponent: root.slotDelegate ? root.slotDelegate : defaultSlot

                function value(name, fallbackValue) {
                    if (entry && typeof entry === "object" && entry[name] !== undefined)
                        return entry[name]
                    return fallbackValue
                }

                function syncItem() {
                    if (!item)
                        return
                    if (item.entry !== undefined)
                        item.entry = entry
                    if (item.slotIndex !== undefined)
                        item.slotIndex = index
                    if (!root.slotDelegate) {
                        item.text = String(value("text", index + 1))
                        item.detail = String(value("detail", ""))
                        item.state = Number(value("state", LfSlotView.Empty))
                        item.selected = Boolean(value("selected", false))
                    }
                }

                onLoaded: syncItem()
                onEntryChanged: syncItem()

                Component {
                    id: defaultSlot
                    LfSlotView {
                        width: root.slotWidth
                        height: root.slotHeight
                        onClicked: root.slotClicked(slotLoader.index, slotLoader.entry)
                    }
                }
            }
        }
    }
}
