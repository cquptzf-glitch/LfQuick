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
}
