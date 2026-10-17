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
    property color badgeColor: Tokens.accent
    property color textColor: Tokens.onAccent
    property int horizontalPadding: 8

    readonly property string displayText: value === undefined || value === null ? "" : String(value)

    implicitWidth: 100
    implicitHeight: Tokens.controlHeight

    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: badgeText.implicitWidth + root.horizontalPadding * 2
        height: Math.max(22, badgeText.implicitHeight + 6)
        radius: height / 2
        color: root.selected ? Tokens.onAccent : root.badgeColor
        opacity: root.enabled ? 1.0 : 0.45
        visible: root.displayText.length > 0

        Text {
            id: badgeText
            anchors.centerIn: parent
            text: root.displayText
            color: root.selected ? Tokens.accent : root.textColor
            font.pixelSize: Tokens.fontSizeCaption
            font.bold: true
        }
    }
}
