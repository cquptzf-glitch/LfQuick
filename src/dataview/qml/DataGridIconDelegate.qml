import QtQuick
import QtQuick.Controls
import LfQuick

Item {
    id: root

    property var value: ""
    property url iconSource: value === undefined || value === null ? "" : String(value)
    property string fallbackText: ""
    property real iconSize: 20
    property bool selected: false
    property bool current: false
    property int row: -1
    property int modelColumn: -1
    property var grid: null

    implicitWidth: 72
    implicitHeight: Tokens.controlHeight

    Image {
        id: icon
        anchors.centerIn: parent
        width: root.iconSize
        height: root.iconSize
        source: root.iconSource
        fillMode: Image.PreserveAspectFit
        visible: root.iconSource.toString().length > 0 && status !== Image.Error
    }

    Label {
        anchors.centerIn: parent
        visible: !icon.visible
        text: root.fallbackText.length > 0
              ? root.fallbackText
              : (root.value === undefined || root.value === null ? "" : String(root.value))
        color: root.selected ? Tokens.onScrim : Tokens.textPrimary
        elide: Text.ElideRight
    }
}
