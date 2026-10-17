import QtQuick

Item {
    id: root

    enum State { Off, Neutral, Info, Success, Warning, Error, Critical }

    property int state: LfStateLight.Neutral
    property bool active: true
    property bool pulse: false
    property int diameter: Tokens.indicatorSize
    property color customColor: "transparent"

    readonly property color stateColor: customColor.a > 0 ? customColor
        : state === LfStateLight.Info ? Tokens.info
        : state === LfStateLight.Success ? Tokens.success
        : state === LfStateLight.Warning ? Tokens.warning
        : state === LfStateLight.Error ? Tokens.error
        : state === LfStateLight.Critical ? Tokens.critical
        : state === LfStateLight.Off ? Tokens.textDisabled
        : Tokens.textSecondary

    implicitWidth: diameter
    implicitHeight: diameter
    Accessible.role: Accessible.Indicator
    Accessible.name: qsTr("State indicator")

    Rectangle {
        id: lamp
        anchors.fill: parent
        radius: width / 2
        color: root.stateColor
        opacity: root.active ? 1.0 : 0.35
        border.width: 1
        border.color: Tokens.borderStrong

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: root.pulse && root.active && Theme.animationEnabled
            NumberAnimation { from: 1.0; to: 0.45; duration: 520; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.45; to: 1.0; duration: 520; easing.type: Easing.InOutSine }
        }
    }
}
