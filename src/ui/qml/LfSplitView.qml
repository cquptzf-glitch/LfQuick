import QtQuick
import QtQuick.Controls

SplitView {
    id: root

    property int handleThickness: Theme.density === Theme.Touch ? 12 : 7
    property color handleColor: Tokens.border
    property color handleHoverColor: Tokens.borderStrong
    property color handlePressedColor: Tokens.accent

    handle: Rectangle {
        implicitWidth: root.orientation === Qt.Horizontal ? root.handleThickness : 1
        implicitHeight: root.orientation === Qt.Vertical ? root.handleThickness : 1
        color: "transparent"

        Rectangle {
            anchors.centerIn: parent
            width: root.orientation === Qt.Horizontal ? 1 : parent.width
            height: root.orientation === Qt.Horizontal ? parent.height : 1
            color: SplitHandle.pressed ? root.handlePressedColor
                 : SplitHandle.hovered ? root.handleHoverColor
                 : root.handleColor
        }
    }
}
