pragma Singleton
import QtQuick

QtObject {
    readonly property int caption: Theme.density === Theme.Touch ? 14 : 12
    readonly property int body: Theme.density === Theme.Touch ? 16 : 14
    readonly property int bodyStrong: body
    readonly property int subtitle: Theme.density === Theme.Touch ? 18 : 16
    readonly property int title: Theme.density === Theme.Touch ? 24 : 20
    readonly property int titleLarge: Theme.density === Theme.Touch ? 30 : 28
    readonly property int display: Theme.density === Theme.Touch ? 38 : 34

    readonly property int weightRegular: Font.Normal
    readonly property int weightMedium: Font.Medium
    readonly property int weightSemibold: Font.DemiBold
    readonly property int weightBold: Font.Bold

    readonly property real captionLineHeight: 1.25
    readonly property real bodyLineHeight: 1.35
    readonly property real titleLineHeight: 1.2
}
