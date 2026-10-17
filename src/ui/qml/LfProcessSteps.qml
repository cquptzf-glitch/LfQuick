import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property var steps: []
    property int orientation: Qt.Horizontal
    property int currentIndex: -1
    property bool showLabels: true

    signal stepClicked(int index, var step)

    implicitWidth: orientation === Qt.Horizontal ? 420 : 180
    implicitHeight: orientation === Qt.Horizontal ? 82 : 320
    padding: Tokens.spacingSmall

    function stepState(index, step) {
        if (step && typeof step === "object" && step.state !== undefined)
            return String(step.state).toLowerCase()
        if (index < currentIndex)
            return "done"
        if (index === currentIndex)
            return "active"
        return "pending"
    }

    function stateColor(state) {
        if (state === "done" || state === "success")
            return Tokens.success
        if (state === "active" || state === "running")
            return Tokens.accent
        if (state === "warning")
            return Tokens.warning
        if (state === "error" || state === "failed")
            return Tokens.error
        return Tokens.surfaceTertiary
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: "transparent"
    }

    contentItem: Item {
        id: canvas

        Repeater {
            model: root.steps

            delegate: Item {
                id: stepItem
                required property int index
                required property var modelData

                readonly property string stepState: root.stepState(index, modelData)
                readonly property bool horizontal: root.orientation === Qt.Horizontal
                readonly property int count: Math.max(1, root.steps.length || 0)

                x: horizontal ? index * canvas.width / count : 0
                y: horizontal ? 0 : index * canvas.height / count
                width: horizontal ? canvas.width / count : canvas.width
                height: horizontal ? canvas.height : canvas.height / count

                Rectangle {
                    visible: stepItem.index < stepItem.count - 1
                    color: stepItem.stepState === "done" || stepItem.stepState === "success"
                           ? Tokens.success : Tokens.borderStrong
                    x: stepItem.horizontal ? stepItem.width / 2 : stepItem.width / 2 - 1
                    y: stepItem.horizontal ? Tokens.indicatorSize / 2 : stepItem.height / 2
                    width: stepItem.horizontal ? stepItem.width : 2
                    height: stepItem.horizontal ? 2 : stepItem.height
                }

                Rectangle {
                    id: marker
                    width: Tokens.indicatorSize
                    height: width
                    radius: width / 2
                    anchors.horizontalCenter: stepItem.horizontal ? parent.horizontalCenter : undefined
                    anchors.verticalCenter: stepItem.horizontal ? undefined : parent.verticalCenter
                    x: stepItem.horizontal ? undefined : Math.max(0, stepItem.width * 0.16 - width / 2)
                    y: stepItem.horizontal ? 0 : undefined
                    color: root.stateColor(stepItem.stepState)
                    border.width: stepItem.stepState === "pending" ? 2 : 0
                    border.color: Tokens.borderStrong
                }

                Text {
                    visible: root.showLabels
                    text: stepItem.modelData && typeof stepItem.modelData === "object"
                          && stepItem.modelData.label !== undefined
                          ? String(stepItem.modelData.label)
                          : String(stepItem.index + 1)
                    color: stepItem.stepState === "pending" ? Tokens.textSecondary : Tokens.textPrimary
                    font.pixelSize: Tokens.fontSizeCaption
                    font.bold: stepItem.stepState === "active" || stepItem.stepState === "running"
                    horizontalAlignment: stepItem.horizontal ? Text.AlignHCenter : Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    anchors.left: stepItem.horizontal ? parent.left : marker.right
                    anchors.leftMargin: stepItem.horizontal ? 2 : Tokens.spacingSmall
                    anchors.right: parent.right
                    anchors.top: stepItem.horizontal ? marker.bottom : parent.top
                    anchors.topMargin: stepItem.horizontal ? Tokens.spacingSmall : 0
                    anchors.bottom: stepItem.horizontal ? parent.bottom : parent.bottom
                }

                TapHandler {
                    onTapped: root.stepClicked(stepItem.index, stepItem.modelData)
                }
            }
        }
    }
}
