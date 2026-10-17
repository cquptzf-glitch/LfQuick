import QtQuick

Rectangle {
    id: root

    property bool running: true
    property color baseColor: Tokens.surfaceTertiary
    property real minimumOpacity: Theme.dark ? 0.45 : 0.55
    property real maximumOpacity: Theme.dark ? 0.85 : 1.0

    implicitWidth: 120
    implicitHeight: Tokens.controlHeight
    radius: Tokens.radiusSmall
    color: baseColor
    opacity: running ? minimumOpacity : maximumOpacity

    SequentialAnimation on opacity {
        running: root.running && Theme.animationEnabled
        loops: Animation.Infinite

        NumberAnimation {
            from: root.minimumOpacity
            to: root.maximumOpacity
            duration: Math.max(240, Tokens.animationNormal * 3)
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            from: root.maximumOpacity
            to: root.minimumOpacity
            duration: Math.max(240, Tokens.animationNormal * 3)
            easing.type: Easing.InOutSine
        }
    }
}
