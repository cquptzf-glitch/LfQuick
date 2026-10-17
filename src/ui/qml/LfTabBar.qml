import QtQuick
import QtQuick.Controls as Controls

Controls.TabBar {
    id: control

    implicitHeight: Math.max(Tokens.controlHeight + Units.xs, contentItem.implicitHeight)
    spacing: Units.xs

    contentItem: ListView {
        model: control.contentModel
        currentIndex: control.currentIndex
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.HorizontalFlick
        snapMode: ListView.SnapToItem
        highlightMoveDuration: Motion.fast
    }

    background: Rectangle {
        color: "transparent"

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Tokens.border
        }
    }
}
