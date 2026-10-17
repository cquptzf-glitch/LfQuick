import QtQuick

Shortcut {
    id: root

    property var target: null
    property HelpContextRegistry registry: null
    property bool requireActiveFocus: true

    sequence: "F1"
    autoRepeat: false
    enabled: registry !== null
             && target !== null
             && (!requireActiveFocus
                 || target.activeFocus === undefined
                 || target.activeFocus === true)

    onActivated: {
        if (registry && target)
            registry.requestHelpFor(target)
    }
}
