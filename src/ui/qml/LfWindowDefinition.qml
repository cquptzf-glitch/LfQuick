import QtQml

QtObject {
    id: root

    required property WindowRuntime runtime
    required property string windowId
    required property url source

    property int openPolicy: WindowRuntime.SingleInstance
    property int ownerClosePolicy: WindowRuntime.CloseWithOwner
    property var options: ({})
    property bool autoRegister: true

    readonly property bool registered: d.registered
    readonly property string errorString: d.errorString

    signal registrationFailed(string message)

    function registerNow() {
        if (!runtime) {
            d.errorString = qsTr("WindowRuntime is not available")
            registrationFailed(d.errorString)
            return false
        }
        if (d.registered)
            return true

        const accepted = runtime.registerWindow(
            windowId, source, openPolicy, ownerClosePolicy, options)
        if (!accepted) {
            d.errorString = runtime.lastError
            registrationFailed(d.errorString)
            return false
        }

        d.registered = true
        d.errorString = ""
        return true
    }

    function unregisterNow() {
        if (!d.registered)
            return true
        if (!runtime)
            return false
        if (!runtime.unregisterWindow(windowId)) {
            d.errorString = runtime.lastError
            return false
        }
        d.registered = false
        d.errorString = ""
        return true
    }

    Component.onCompleted: {
        if (autoRegister)
            registerNow()
    }

    // Do not force-unregister on destruction. Runtime intentionally owns the
    // descriptor after successful registration, and unregistering can be
    // rejected while live instances still exist.
    property QtObject _private: QtObject {
        id: d
        property bool registered: false
        property string errorString: ""
    }
}
