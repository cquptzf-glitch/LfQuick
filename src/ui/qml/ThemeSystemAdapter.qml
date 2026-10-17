import QtQml

QtObject {
    id: root

    // Duck-typed contract: appearance should expose `dark`, `accentColor`
    // and preferably an `appearanceChanged` signal. This keeps LfQuick.Ui
    // independent from LfQuick.Platform while allowing them to compose.
    property var appearance: null
    property bool syncAccent: true
    property bool enabled: true

    function sync() {
        if (!enabled || !appearance)
            return

        if (appearance.dark !== undefined)
            Theme.systemDark = Boolean(appearance.dark)
        if (syncAccent && appearance.accentColor !== undefined
                && String(appearance.accentColor).length > 0) {
            Theme.accentColor = appearance.accentColor
        }
    }

    onAppearanceChanged: sync()
    onSyncAccentChanged: sync()
    onEnabledChanged: sync()
    Component.onCompleted: sync()

    Connections {
        target: root.appearance
        ignoreUnknownSignals: true
        function onAppearanceChanged() { root.sync() }
        function onDarkChanged() { root.sync() }
        function onAccentColorChanged() { root.sync() }
    }
}
