import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root

    property bool frameless: true
    property string appBarSubtitle: ""
    property url appBarIconSource
    property bool showMinimizeButton: true
    property bool showMaximizeButton: true
    property bool showCloseButton: true
    readonly property var windowController: controller
    readonly property var windowCapabilities: capabilities

    flags: frameless ? (Qt.Window | Qt.FramelessWindowHint) : Qt.Window
    color: Tokens.surface

    WindowController {
        id: controller
        window: root
    }

    WindowCapabilities {
        id: capabilities
        window: root
    }

    header: LfAppBar {
        controller: root.windowController
        title: root.title
        subtitle: root.appBarSubtitle
        iconSource: root.appBarIconSource
        showMinimizeButton: root.showMinimizeButton
        showMaximizeButton: root.showMaximizeButton
        showCloseButton: root.showCloseButton
    }
}
