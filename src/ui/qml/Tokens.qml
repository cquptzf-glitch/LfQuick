pragma Singleton
import QtQuick

QtObject {
    // Semantic surfaces/text. Component QML should consume these values
    // instead of embedding brand-specific colors locally.
    readonly property color surface: Theme.dark ? "#202020" : "#FFFFFF"
    readonly property color surfaceSecondary: Theme.dark ? "#2B2B2B" : "#F5F5F5"
    readonly property color surfaceTertiary: Theme.dark ? "#353535" : "#EEEEEE"
    readonly property color textPrimary: Theme.dark ? "#FFFFFF" : "#1A1A1A"
    readonly property color textSecondary: Theme.dark ? "#C8C8C8" : "#5E5E5E"
    readonly property color textDisabled: Theme.dark ? "#777777" : "#9A9A9A"
    readonly property color border: Theme.dark ? "#515151" : "#D6D6D6"
    readonly property color borderStrong: Theme.dark ? "#717171" : "#9A9A9A"

    readonly property color accent: Theme.accentColor
    readonly property color accentHover: Qt.lighter(Theme.accentColor, Theme.dark ? 1.18 : 1.08)
    readonly property color accentPressed: Qt.darker(Theme.accentColor, 1.12)
    readonly property color onAccent: "#FFFFFF"

    readonly property color controlFill: Theme.dark ? "#333333" : "#FFFFFF"
    readonly property color controlFillHover: Theme.dark ? "#3D3D3D" : "#F7F7F7"
    readonly property color controlFillPressed: Theme.dark ? "#292929" : "#ECECEC"
    readonly property color controlFillDisabled: Theme.dark ? "#292929" : "#F3F3F3"
    readonly property color selection: Qt.rgba(accent.r, accent.g, accent.b, Theme.dark ? 0.55 : 0.28)
    readonly property color focusRing: accent

    readonly property color scrim: Qt.rgba(0, 0, 0, 0.38)
    readonly property color onScrim: "#FFFFFF"
    readonly property color success: "#0F7B0F"
    readonly property color warning: "#9D5D00"
    readonly property color error: "#C42B1C"
    readonly property color danger: error
    readonly property color critical: "#A80000"
    readonly property color info: accent

    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    readonly property int spacingXSmall: 4
    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 20
    readonly property int spacingXLarge: 28

    readonly property int controlHeight: Theme.density === Theme.Touch ? 44
                                         : Theme.density === Theme.Compact ? 28
                                         : 32
    readonly property int controlPaddingHorizontal: Theme.density === Theme.Touch ? 16 : 12
    readonly property int controlSpacing: Theme.density === Theme.Touch ? 10 : 8
    readonly property int indicatorSize: Theme.density === Theme.Touch ? 22 : 18
    readonly property int fontSizeBody: Theme.density === Theme.Touch ? 16 : 14
    readonly property int fontSizeCaption: Theme.density === Theme.Touch ? 14 : 12
    readonly property int animationFast: Theme.animationEnabled ? 90 : 0
    readonly property int animationNormal: Theme.animationEnabled ? 160 : 0
}
