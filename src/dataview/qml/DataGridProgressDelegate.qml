import QtQuick
import QtQuick.Controls
import LfQuick

Item {
    id: root

    property var value: 0
    property real from: 0
    property real to: 1
    property bool showText: true
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null

    implicitWidth: 120
    implicitHeight: Tokens.controlHeight

    function numericValue() {
        var number = Number(root.value)
        if (isNaN(number))
            return root.from
        return Math.max(root.from, Math.min(root.to, number))
    }

    ProgressBar {
        id: bar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        from: root.from
        to: root.to
        value: root.numericValue()
    }

    Label {
        anchors.centerIn: parent
        visible: root.showText
        text: root.to === root.from ? "" : Math.round((bar.value - root.from) * 100 / (root.to - root.from)) + "%"
        color: Tokens.textPrimary
        font.pixelSize: 11
    }
}
