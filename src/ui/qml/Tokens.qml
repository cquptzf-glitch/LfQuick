pragma Singleton
import QtQuick

QtObject {
    readonly property color surface: Theme.dark ? "#202020" : "#FFFFFF"
    readonly property color surfaceSecondary: Theme.dark ? "#2B2B2B" : "#F5F5F5"
    readonly property color textPrimary: Theme.dark ? "#FFFFFF" : "#1A1A1A"
    readonly property color textSecondary: Theme.dark ? "#C8C8C8" : "#5E5E5E"
    readonly property color border: Theme.dark ? "#515151" : "#D6D6D6"
    readonly property color accent: Theme.accentColor
    readonly property color scrim: Qt.rgba(0, 0, 0, 0.38)
    readonly property color onScrim: "#FFFFFF"
    readonly property color success: "#0F7B0F"
    readonly property color warning: "#9D5D00"
    readonly property color error: "#C42B1C"
    readonly property color critical: "#A80000"

    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 20
    readonly property int controlHeight: Theme.density === Theme.Touch ? 44 : 32
}
