import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property var model: []
    property int currentIndex: 0
    property bool interactive: true

    signal stepActivated(int index, var value)

    readonly property int stepCount: repeater.count

    function valueAt(index) {
        if (!model)
            return undefined
        if (typeof model.get === "function")
            return model.get(index)
        return model[index]
    }

    function labelFor(value) {
        if (typeof value === "string")
            return value
        return value && value.text !== undefined ? String(value.text) : String(value)
    }

    function activate(index) {
        if (!interactive || stepCount <= 0)
            return
        const target = Math.max(0, Math.min(stepCount - 1, index))
        if (target === currentIndex)
            return
        currentIndex = target
        stepActivated(target, valueAt(target))
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight

    contentItem: Row {
        id: row
        spacing: 0

        Repeater {
            id: repeater
            model: root.model

            delegate: Item {
                id: step
                required property int index

                readonly property var value: root.valueAt(index)
                readonly property bool completed: index < root.currentIndex
                readonly property bool current: index === root.currentIndex
                readonly property string label: root.labelFor(value)

                width: Math.max(96, labelText.implicitWidth + Units.lg * 2)
                height: marker.height + Units.sm + labelText.implicitHeight

                Rectangle {
                    id: connector
                    anchors.left: marker.horizontalCenter
                    anchors.right: parent.right
                    anchors.verticalCenter: marker.verticalCenter
                    height: 2
                    visible: step.index < repeater.count - 1
                    color: step.completed ? Tokens.accent : Tokens.border
                }

                Rectangle {
                    id: marker
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Units.controlHeight
                    height: width
                    radius: width / 2
                    color: step.completed || step.current ? Tokens.accent : Tokens.surface
                    border.width: 1
                    border.color: step.completed || step.current ? Tokens.accent : Tokens.borderStrong

                    Text {
                        anchors.centerIn: parent
                        text: step.completed ? "✓" : String(step.index + 1)
                        color: step.completed || step.current ? Tokens.onAccent : Tokens.textSecondary
                        font.pixelSize: Typography.caption
                        font.weight: Typography.weightSemibold
                    }
                }

                Text {
                    id: labelText
                    anchors.top: marker.bottom
                    anchors.topMargin: Units.sm
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - Units.md * 2
                    text: step.label
                    color: step.current ? Tokens.textPrimary : Tokens.textSecondary
                    font.pixelSize: Typography.caption
                    font.weight: step.current ? Typography.weightSemibold : Typography.weightRegular
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                TapHandler {
                    enabled: root.interactive
                    onTapped: root.activate(step.index)
                }
            }
        }
    }

    background: Item { }
}
