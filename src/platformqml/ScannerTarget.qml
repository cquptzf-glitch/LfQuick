import QtQml

QtObject {
    id: root

    property InputRouter router: null
    property var target: null
    property string targetId: ""
    property bool enabled: true
    property bool acceptScanner: true
    property int priority: 0
    property bool autoActive: true
    property bool manualActive: false
    readonly property bool active: autoActive
                                   ? Boolean(target && target.activeFocus === true)
                                   : manualActive
    readonly property InputTarget inputTarget: registration

    property var _registeredRouter: null

    signal inputReceived(string value, var metadata)

    function updateRegistration() {
        if (_registeredRouter && _registeredRouter !== router)
            _registeredRouter.unregisterTarget(registration)
        _registeredRouter = null
        if (router && router.registerTarget(registration))
            _registeredRouter = router
    }

    onRouterChanged: updateRegistration()
    Component.onCompleted: updateRegistration()
    Component.onDestruction: {
        if (_registeredRouter)
            _registeredRouter.unregisterTarget(registration)
    }

    Connections {
        target: root._registeredRouter
        ignoreUnknownSignals: true

        function onDestroyed() {
            // InputRouter removes registered targets as part of its own
            // destruction. Only clear the QML snapshot here so later teardown
            // never calls back into a dying router.
            root._registeredRouter = null
        }
    }

    InputTarget {
        id: registration
        targetId: root.targetId
        target: root.target
        enabled: root.enabled
        active: root.active
        acceptScanner: root.acceptScanner
        priority: root.priority
        onInputReceived: function(value, metadata) {
            root.inputReceived(value, metadata)
        }
    }
}
