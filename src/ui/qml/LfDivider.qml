import QtQuick

Item {
    id: root

    enum Orientation {
        Horizontal,
        Vertical
    }

    property int orientation: LfDivider.Horizontal
    property color color: Tokens.border
    property real thickness: 1

    readonly property bool horizontal: orientation === LfDivider.Horizontal

    implicitWidth: horizontal ? Units.xxl : thickness
    implicitHeight: horizontal ? thickness : Units.xxl

    Rectangle {
        anchors.centerIn: parent
        width: root.horizontal ? parent.width : root.thickness
        height: root.horizontal ? root.thickness : parent.height
        color: root.color
    }
}
