pragma Singleton
import QtQuick

QtObject {
    readonly property bool compact: Theme.density === Theme.Compact
    readonly property bool touch: Theme.density === Theme.Touch
    readonly property bool comfortable: Theme.density === Theme.Comfortable

    readonly property int xxs: 2
    readonly property int xs: compact ? 3 : 4
    readonly property int sm: compact ? 4 : 6
    readonly property int md: touch ? 14 : (comfortable ? 14 : 12)
    readonly property int lg: touch ? 24 : (compact ? 16 : 20)
    readonly property int xl: touch ? 32 : 28
    readonly property int xxl: touch ? 40 : 36

    readonly property int controlHeight: touch ? 44 : (compact ? 28 : (comfortable ? 36 : 32))
    readonly property int controlPaddingHorizontal: touch ? 16 : (compact ? 10 : 12)
    readonly property int controlPaddingVertical: touch ? 10 : (compact ? 4 : 6)
    readonly property int controlGap: touch ? 10 : (compact ? 6 : 8)
    readonly property int indicatorSize: touch ? 22 : (compact ? 16 : 18)
    readonly property int iconSize: touch ? 22 : 18

    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    readonly property int radiusRound: 999
}
