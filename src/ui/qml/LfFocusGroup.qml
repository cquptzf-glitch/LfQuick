import QtQuick

FocusScope {
    id: root

    property int navigationMode: {
        switch (Lf.focusPolicy) {
        case Lf.Explicit: return FocusNavigator.Explicit
        case Lf.Geometric: return FocusNavigator.Geometric
        case Lf.Grid: return FocusNavigator.Grid
        case Lf.Native: return FocusNavigator.Native
        default: return FocusNavigator.Native
        }
    }
    property bool wrap: true
    property real rowTolerance: 8
    property bool handleTab: true
    property bool handleArrows: navigationMode === FocusNavigator.Geometric
                                || navigationMode === FocusNavigator.Grid
    readonly property alias navigator: focusNavigator
    readonly property alias manager: focusManager

    function managerMode() {
        switch (root.navigationMode) {
        case FocusNavigator.Explicit: return FocusWalker.Explicit
        case FocusNavigator.Geometric: return FocusWalker.Geometric
        case FocusNavigator.Grid: return FocusWalker.Grid
        default: return FocusWalker.Native
        }
    }

    FocusManager {
        id: focusManager
        scope: root
        mode: root.managerMode()
        wrap: root.wrap
    }

    Binding {
        target: focusManager.walker
        property: "rowTolerance"
        value: root.rowTolerance
    }

    Binding {
        target: focusManager.walker
        property: "layoutDirection"
        value: Qt.application.layoutDirection
    }

    // Compatibility object for existing pages/diagnostics that still bind to
    // FocusNavigator. Keyboard handling below uses FocusManager/FocusWalker.
    FocusNavigator {
        id: focusNavigator
        scope: root
        mode: root.navigationMode
        wrap: root.wrap
        rowTolerance: root.rowTolerance
    }

    Keys.onTabPressed: function(event) {
        if (!root.handleTab)
            return
        event.accepted = focusManager.moveNext()
    }

    Keys.onBacktabPressed: function(event) {
        if (!root.handleTab)
            return
        event.accepted = focusManager.movePrevious()
    }

    Keys.onLeftPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusManager.moveLeft()
    }

    Keys.onRightPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusManager.moveRight()
    }

    Keys.onUpPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusManager.moveUp()
    }

    Keys.onDownPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusManager.moveDown()
    }
}
