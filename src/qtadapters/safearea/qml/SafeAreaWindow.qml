import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root

    property bool automaticSafeAreaPadding: true
    property real additionalTop: 0
    property real additionalLeft: 0
    property real additionalRight: 0
    property real additionalBottom: 0

    readonly property real safeTop: root.SafeArea.margins.top
    readonly property real safeLeft: root.SafeArea.margins.left
    readonly property real safeRight: root.SafeArea.margins.right
    readonly property real safeBottom: root.SafeArea.margins.bottom

    SafeArea.additionalMargins.top: Math.max(0, root.additionalTop)
    SafeArea.additionalMargins.left: Math.max(0, root.additionalLeft)
    SafeArea.additionalMargins.right: Math.max(0, root.additionalRight)
    SafeArea.additionalMargins.bottom: Math.max(0, root.additionalBottom)

    topPadding: automaticSafeAreaPadding ? undefined : 0
    leftPadding: automaticSafeAreaPadding ? undefined : 0
    rightPadding: automaticSafeAreaPadding ? undefined : 0
    bottomPadding: automaticSafeAreaPadding ? undefined : 0
}
