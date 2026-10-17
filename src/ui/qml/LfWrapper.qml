import QtQuick

Item {
    id: root

    default property alias contentData: content.data
    readonly property alias contentItem: content

    property real minimumWidth: 0
    property real minimumHeight: 0

    implicitWidth: Math.max(minimumWidth, content.childrenRect.width)
    implicitHeight: Math.max(minimumHeight, content.childrenRect.height)

    Item {
        id: content
        anchors.fill: parent
    }
}
