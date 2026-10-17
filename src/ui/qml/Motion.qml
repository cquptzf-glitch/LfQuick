pragma Singleton
import QtQuick

QtObject {
    readonly property bool enabled: Theme.animationEnabled
    readonly property int instant: 0
    readonly property int fast: enabled ? 90 : 0
    readonly property int normal: enabled ? 160 : 0
    readonly property int slow: enabled ? 240 : 0
    readonly property int emphasized: enabled ? 320 : 0

    readonly property int standardEasing: Easing.OutCubic
    readonly property int enterEasing: Easing.OutCubic
    readonly property int exitEasing: Easing.InCubic
    readonly property int emphasizedEasing: Easing.OutBack
}
