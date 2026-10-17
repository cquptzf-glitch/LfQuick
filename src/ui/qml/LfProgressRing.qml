import QtQuick
import QtQuick.Controls as Controls

Controls.BusyIndicator {
    id: control

    property color indicatorColor: Tokens.accent

    implicitWidth: Tokens.controlHeight
    implicitHeight: Tokens.controlHeight

    contentItem: Item {
        id: spinner
        implicitWidth: control.implicitWidth
        implicitHeight: control.implicitHeight
        opacity: control.enabled ? 1.0 : 0.45
        visible: control.running

        Repeater {
            model: 8

            Rectangle {
                required property int index
                readonly property real angle: index * Math.PI * 2 / 8
                readonly property real orbit: Math.max(2, Math.min(spinner.width, spinner.height) / 2 - width)
                width: Math.max(2, Math.round(Math.min(spinner.width, spinner.height) / 9))
                height: width
                radius: width / 2
                x: spinner.width / 2 + Math.cos(angle) * orbit - width / 2
                y: spinner.height / 2 + Math.sin(angle) * orbit - height / 2
                color: control.indicatorColor
                opacity: 0.2 + (index + 1) / 10.0
            }
        }

        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: Theme.animationEnabled ? 900 : 1
            loops: Animation.Infinite
            running: control.running && control.visible
        }
    }
}
