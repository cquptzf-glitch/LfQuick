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
    readonly property string registeredWindowId: d.registeredWindowId
    readonly property string errorString: d.errorString

    signal registrationFailed(string message)

    function registerNow() {
        if (!runtime) {
            d.errorString = qsTr("WindowRuntime is not available")
            registrationFailed(d.errorString)
            return false
        }

        if (d.registered) {
            if (d.registeredRuntime === runtime
                    && d.registeredWindowId === windowId)
                return true

            d.errorString = qsTr("Window definition is already registered as '%1'")
                    .arg(d.registeredWindowId)
            registrationFailed(d.errorString)
            return false
        }

        const accepted = runtime.registerWindow(
            windowId, source, openPolicy, ownerClosePolicy, options)
        if (!accepted) {
            d.errorString = runtime.lastError
            registrationFailed(d.errorString)
            return false
        }

        d.registered = true
        d.registeredRuntime = runtime
        d.registeredWindowId = windowId
        d.errorString = ""
        return true
    }

    function unregisterNow() {
        if (!d.registered)
            return true

        const targetRuntime = d.registeredRuntime
        const targetWindowId = d.registeredWindowId
        if (!targetRuntime) {
            d.errorString = qsTr("Registered WindowRuntime is no longer available")
            return false
        }
        if (!targetRuntime.unregisterWindow(targetWindowId)) {
            d.errorString = targetRuntime.lastError
            return false
        }

        d.registered = false
        d.registeredRuntime = null
        d.registeredWindowId = ""
        d.errorString = ""
        return true
    }

    Component.onCompleted: {
        if (autoRegister)
            registerNow()
    }

    // Do not force-unregister on destruction. Runtime intentionally owns the
    // descriptor after successful registration, and unregistering can be
    // rejected while live instances still exist. We only remember the exact
    // registration target so an explicit unregister remains deterministic if
    // this declarative object's input properties later change.
    property QtObject _private: QtObject {
        id: d
        property bool registered: false
        property WindowRuntime registeredRuntime: null
        property string registeredWindowId: ""
        property string errorString: ""
    }
}
