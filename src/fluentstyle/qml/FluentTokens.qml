pragma Singleton
import QtQuick

QtObject {
    property bool dark: false
    property color accent: "#0067C0"

    readonly property color surface: dark ? "#202020" : "#FFFFFF"
    readonly property color surfaceSecondary: dark ? "#2B2B2B" : "#F7F7F7"
    readonly property color text: dark ? "#FFFFFF" : "#1B1B1B"
    readonly property color textSecondary: dark ? "#C8C8C8" : "#606060"
    readonly property color border: dark ? "#5A5A5A" : "#D0D0D0"
    readonly property color fill: dark ? "#323232" : "#FFFFFF"
    readonly property color hover: dark ? "#3A3A3A" : "#F3F3F3"
    readonly property color pressed: dark ? "#292929" : "#E9E9E9"
    readonly property color disabled: dark ? "#292929" : "#F0F0F0"
    readonly property color onAccent: "#FFFFFF"

    readonly property int radius: 4
    readonly property int controlHeight: 32
    readonly property int spacing: 8
    readonly property int padding: 12
    readonly property int fontSize: 14
}
