import QtQuick

Item {
    id: root

    property real padding: 0
    property real leftPadding: padding
    property real rightPadding: padding
    property real topPadding: padding
    property real bottomPadding: padding

    default property alias contentData: content.data
    readonly property alias contentItem: content

    implicitWidth: content.childrenRect.width + leftPadding + rightPadding
    implicitHeight: content.childrenRect.height + topPadding + bottomPadding

    Item {
        id: content

        x: root.leftPadding
        y: root.topPadding
        width: Math.max(0, root.width - root.leftPadding - root.rightPadding)
        height: Math.max(0, root.height - root.topPadding - root.bottomPadding)
    }
}
