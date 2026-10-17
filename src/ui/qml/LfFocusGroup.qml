import QtQuick

FocusScope {
    id: root

    property int navigationMode: FocusNavigator.Native
    property bool wrap: true
    property real rowTolerance: 8
    property bool handleTab: true
    property bool handleArrows: navigationMode === FocusNavigator.Geometric
                                || navigationMode === FocusNavigator.Grid
    readonly property alias navigator: focusNavigator

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
        event.accepted = focusNavigator.moveNext()
    }

    Keys.onBacktabPressed: function(event) {
        if (!root.handleTab)
            return
        event.accepted = focusNavigator.movePrevious()
    }

    Keys.onLeftPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusNavigator.moveLeft()
    }

    Keys.onRightPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusNavigator.moveRight()
    }

    Keys.onUpPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusNavigator.moveUp()
    }

    Keys.onDownPressed: function(event) {
        if (root.handleArrows)
            event.accepted = focusNavigator.moveDown()
    }
}
