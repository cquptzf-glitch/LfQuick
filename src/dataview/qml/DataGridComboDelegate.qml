import QtQuick
import QtQuick.Controls
import LfQuick

Item {
    id: root

    property var value: ""
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null

    implicitWidth: 120
    implicitHeight: Tokens.controlHeight

    Label {
        anchors.fill: parent
        leftPadding: 10
        rightPadding: 26
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: root.value === undefined || root.value === null ? "" : String(root.value)
        color: root.selected ? Tokens.onScrim : Tokens.textPrimary
    }

    Label {
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: "▾"
        color: root.selected ? Tokens.onScrim : Tokens.textSecondary
    }
}
