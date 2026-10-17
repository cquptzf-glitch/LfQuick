import QtQuick
import LfQuick

Item {
    id: root

    property var value: ""
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null
    property string text: value === undefined || value === null ? "" : String(value)
    property bool destructive: false
    property bool highlighted: false
    property bool actionEnabled: true

    signal actionTriggered(int row, int modelColumn, var value)

    implicitWidth: 110
    implicitHeight: Tokens.controlHeight

    LfButton {
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        height: Math.max(24, parent.height - 6)
        text: root.text
        enabled: root.enabled && root.actionEnabled
        destructive: root.destructive
        highlighted: root.highlighted
        onClicked: root.actionTriggered(root.row, root.modelColumn, root.value)
    }
}
