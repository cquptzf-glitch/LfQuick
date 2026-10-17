import QtQuick

Item {
    id: root

    default property alias contentData: content.data
    property real additionalTop: 0
    property real additionalLeft: 0
    property real additionalRight: 0
    property real additionalBottom: 0
    property bool applyTop: true
    property bool applyLeft: true
    property bool applyRight: true
    property bool applyBottom: true

    readonly property real safeTop: root.SafeArea.margins.top
    readonly property real safeLeft: root.SafeArea.margins.left
    readonly property real safeRight: root.SafeArea.margins.right
    readonly property real safeBottom: root.SafeArea.margins.bottom

    SafeArea.additionalMargins.top: Math.max(0, root.additionalTop)
    SafeArea.additionalMargins.left: Math.max(0, root.additionalLeft)
    SafeArea.additionalMargins.right: Math.max(0, root.additionalRight)
    SafeArea.additionalMargins.bottom: Math.max(0, root.additionalBottom)

    Item {
        id: content
        anchors.fill: parent
        anchors.topMargin: root.applyTop ? root.safeTop : 0
        anchors.leftMargin: root.applyLeft ? root.safeLeft : 0
        anchors.rightMargin: root.applyRight ? root.safeRight : 0
        anchors.bottomMargin: root.applyBottom ? root.safeBottom : 0
    }
}
