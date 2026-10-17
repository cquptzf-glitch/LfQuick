import QtQuick

Item {
    id: root

    default property alias contentData: content.data
    readonly property alias contentItem: content

    clip: true
    implicitWidth: content.childrenRect.width
    implicitHeight: content.childrenRect.height

    Item {
        id: content
        anchors.fill: parent
    }
}
