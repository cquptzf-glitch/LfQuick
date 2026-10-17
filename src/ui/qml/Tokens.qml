pragma Singleton
import QtQuick

QtObject {
    // Semantic surfaces/text. Component QML should consume these values
    // instead of embedding brand-specific colors locally.
    readonly property color surface: Theme.highContrast
                                     ? (Theme.dark ? "#000000" : "#FFFFFF")
                                     : (Theme.dark ? "#202020" : "#FFFFFF")
    readonly property color surfaceSecondary: Theme.highContrast
                                              ? surface
                                              : (Theme.dark ? "#2B2B2B" : "#F5F5F5")
    readonly property color surfaceTertiary: Theme.highContrast
                                             ? surface
                                             : (Theme.dark ? "#353535" : "#EEEEEE")
    readonly property color textPrimary: Theme.dark ? "#FFFFFF" : "#000000"
    readonly property color textSecondary: Theme.highContrast
                                           ? textPrimary
                                           : (Theme.dark ? "#C8C8C8" : "#5E5E5E")
    readonly property color textDisabled: Theme.highContrast
                                          ? (Theme.dark ? "#A0A0A0" : "#606060")
                                          : (Theme.dark ? "#777777" : "#9A9A9A")
    readonly property color border: Theme.highContrast
                                    ? textPrimary
                                    : (Theme.dark ? "#515151" : "#D6D6D6")
    readonly property color borderStrong: Theme.highContrast
                                          ? textPrimary
                                          : (Theme.dark ? "#717171" : "#9A9A9A")

    readonly property color accent: Theme.highContrast ? textPrimary : Theme.accentColor
    readonly property color accentHover: Theme.highContrast
                                         ? accent
                                         : Qt.lighter(Theme.accentColor, Theme.dark ? 1.18 : 1.08)
    readonly property color accentPressed: Theme.highContrast
                                           ? accent
                                           : Qt.darker(Theme.accentColor, 1.12)
    readonly property color onAccent: Theme.highContrast ? surface : "#FFFFFF"

    readonly property color controlFill: Theme.highContrast
                                         ? surface
                                         : (Theme.dark ? "#333333" : "#FFFFFF")
    readonly property color controlFillHover: Theme.highContrast
                                              ? surface
                                              : (Theme.dark ? "#3D3D3D" : "#F7F7F7")
    readonly property color controlFillPressed: Theme.highContrast
                                                ? surface
                                                : (Theme.dark ? "#292929" : "#ECECEC")
    readonly property color controlFillDisabled: Theme.highContrast
                                                 ? surface
                                                 : (Theme.dark ? "#292929" : "#F3F3F3")
    readonly property color selection: Qt.rgba(accent.r, accent.g, accent.b,
                                               Theme.highContrast ? 0.35
                                                                  : (Theme.dark ? 0.55 : 0.28))
    readonly property color focusRing: Theme.highContrast ? textPrimary : accent

    readonly property color scrim: Qt.rgba(0, 0, 0, Theme.highContrast ? 0.65 : 0.38)
    readonly property color onScrim: "#FFFFFF"
    readonly property color success: Theme.highContrast ? textPrimary : "#0F7B0F"
    readonly property color warning: Theme.highContrast ? textPrimary : "#9D5D00"
    readonly property color error: Theme.highContrast ? textPrimary : "#C42B1C"
    readonly property color danger: error
    readonly property color critical: Theme.highContrast ? textPrimary : "#A80000"
    readonly property color info: accent

    // Compatibility aliases. New components can consume Units/Typography/Motion
    // directly while existing controls keep their Tokens.* API unchanged.
    readonly property int radiusSmall: Units.radiusSmall
    readonly property int radiusMedium: Units.radiusMedium
    readonly property int radiusLarge: Units.radiusLarge
    readonly property int spacingXSmall: Units.xs
    readonly property int spacingSmall: Units.sm
    readonly property int spacingMedium: Units.md
    readonly property int spacingLarge: Units.lg
    readonly property int spacingXLarge: Units.xl

    readonly property int controlHeight: Units.controlHeight
    readonly property int controlPaddingHorizontal: Units.controlPaddingHorizontal
    readonly property int controlSpacing: Units.controlGap
    readonly property int indicatorSize: Units.indicatorSize
    readonly property int fontSizeBody: Typography.body
    readonly property int fontSizeCaption: Typography.caption
    readonly property int animationFast: Motion.fast
    readonly property int animationNormal: Motion.normal
}
