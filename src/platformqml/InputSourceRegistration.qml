import QtQml

QtObject {
    id: root

    property InputRouter router: null
    property var source: null
    property var _registeredRouter: null
    property var _registeredSource: null

    readonly property bool registered: _registeredRouter !== null
                                      && _registeredSource !== null

    function updateRegistration() {
        if (_registeredRouter && _registeredSource)
            _registeredRouter.unregisterSourceObject(_registeredSource)

        _registeredRouter = null
        _registeredSource = null

        if (router && source && router.registerSourceObject(source)) {
            _registeredRouter = router
            _registeredSource = source
        }
    }

    onRouterChanged: updateRegistration()
    onSourceChanged: updateRegistration()
    Component.onCompleted: updateRegistration()
    Component.onDestruction: {
        if (_registeredRouter && _registeredSource)
            _registeredRouter.unregisterSourceObject(_registeredSource)
    }

    Connections {
        target: root._registeredRouter
        ignoreUnknownSignals: true

        function onDestroyed() {
            // InputRouter owns the source-destruction bookkeeping. Clear only
            // the QML registration snapshot here so `registered` becomes false
            // without calling back into an object that is being destroyed.
            root._registeredRouter = null
            root._registeredSource = null
        }
    }

    Connections {
        target: root._registeredSource
        ignoreUnknownSignals: true

        function onDestroyed() {
            root._registeredSource = null
        }
    }
}
