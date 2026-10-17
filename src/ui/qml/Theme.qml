pragma Singleton
import QtQuick

QtObject {
    id: root

    enum Mode { Light, Dark, System }
    enum Density { Compact, Standard, Comfortable, Touch }

    property int mode: Theme.System
    property bool systemDark: false
    property bool dark: false
    property color accentColor: "#0067C0"
    property int density: Theme.Standard
    property bool animationEnabled: true
    property bool highContrast: false

    function syncMode() {
        if (mode === Theme.Light)
            dark = false
        else if (mode === Theme.Dark)
            dark = true
        else
            dark = systemDark
    }

    function useLight() {
        mode = Theme.Light
        syncMode()
    }

    function useDark() {
        mode = Theme.Dark
        syncMode()
    }

    function useSystem() {
        mode = Theme.System
        syncMode()
    }

    onModeChanged: syncMode()
    onSystemDarkChanged: {
        if (mode === Theme.System)
            dark = systemDark
    }

    Component.onCompleted: syncMode()
}
