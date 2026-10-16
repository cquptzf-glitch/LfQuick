pragma Singleton
import QtQuick

QtObject {
    id: root

    enum Mode { Light, Dark, System }
    enum Density { Compact, Standard, Comfortable, Touch }

    property int mode: Theme.System
    property bool dark: false
    property color accentColor: "#0067C0"
    property int density: Theme.Standard
    property bool animationEnabled: true
    property bool highContrast: false
}
