import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

// Model entries: { key, label, value, type: "text"|"bool"|"int", readOnly? }.
// Emits edits instead of mutating source; caller validates and updates its model.
Controls.Control {
    id: root

    property var model: []
    property bool readOnly: false
    property int labelWidth: 140

    signal valueEdited(int index, string key, var value)

    readonly property int entryCount: model === null ? 0
        : (typeof model.count === "number" ? model.count
           : (typeof model.length === "number" ? model.length : 0))

    implicitWidth: 380
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
    padding: Units.md

    function entryAt(index) {
        if (!model || index < 0 || index >= entryCount)
            return null
        return typeof model.get === "function" ? model.get(index) : model[index]
    }

    function keyAt(index) {
        const entry = entryAt(index)
        return entry && entry.key !== undefined ? String(entry.key) : String(index)
    }

    function editValue(index, value) {
        const entry = entryAt(index)
        if (!entry || readOnly || entry.readOnly === true)
            return false
        valueEdited(index, keyAt(index), value)
        return true
    }

    contentItem: ColumnLayout {
        spacing: Units.xs

        Repeater {
            model: root.model

            delegate: RowLayout {
                id: entryRow
                required property int index
                readonly property var entry: root.entryAt(index)
                readonly property bool locked: root.readOnly || (entry && entry.readOnly === true)
                readonly property string entryType: entry && entry.type ? String(entry.type) : "text"
                Layout.fillWidth: true
                spacing: Units.md

                Text {
                    Layout.preferredWidth: root.labelWidth
                    text: entryRow.entry && entryRow.entry.label !== undefined
                          ? String(entryRow.entry.label) : root.keyAt(entryRow.index)
                    font.pixelSize: Typography.body
                    color: Tokens.textSecondary
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                Loader {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 100
                    sourceComponent: entryRow.entryType === "bool" ? boolEditor
                        : entryRow.entryType === "int" ? intEditor
                        : entryRow.locked ? valueLabel : textEditor
                }

                Component {
                    id: valueLabel
                    Text {
                        text: entryRow.entry && entryRow.entry.value !== undefined
                              ? String(entryRow.entry.value) : ""
                        color: Tokens.textPrimary
                        font.pixelSize: Typography.body
                        elide: Text.ElideRight
                    }
                }

                Component {
                    id: textEditor
                    LfTextField {
                        text: entryRow.entry && entryRow.entry.value !== undefined
                              ? String(entryRow.entry.value) : ""
                        enabled: !entryRow.locked
                        onEditingFinished: root.editValue(entryRow.index, text)
                    }
                }

                Component {
                    id: intEditor
                    LfNumberInput {
                        from: entryRow.entry && entryRow.entry.from !== undefined
                              ? Number(entryRow.entry.from) : -1000000
                        to: entryRow.entry && entryRow.entry.to !== undefined
                              ? Number(entryRow.entry.to) : 1000000
                        value: entryRow.entry ? Number(entryRow.entry.value) : 0
                        enabled: !entryRow.locked
                        onValueModified: root.editValue(entryRow.index, value)
                    }
                }

                Component {
                    id: boolEditor
                    LfSwitch {
                        checked: entryRow.entry ? Boolean(entryRow.entry.value) : false
                        enabled: !entryRow.locked
                        onToggled: root.editValue(entryRow.index, checked)
                    }
                }
            }
        }
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Tokens.surfaceSecondary
        border.width: 1
        border.color: Tokens.border
    }
}
