import QtQuick
import QtQuick.Controls as Controls

Controls.ProgressBar {
    id: control

    implicitWidth: 180
    implicitHeight: 8

    background: Rectangle {
        implicitWidth: 180
        implicitHeight: 6
        radius: height / 2
        color: Tokens.surfaceTertiary
    }

    contentItem: Item {
        implicitWidth: 180
        implicitHeight: 6
        clip: true

        Rectangle {
            width: control.indeterminate ? parent.width * 0.35 : control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: control.enabled ? Tokens.accent : Tokens.textDisabled

            SequentialAnimation on x {
                running: control.indeterminate && control.visible && Theme.animationEnabled
                loops: Animation.Infinite
                NumberAnimation {
                    from: -control.contentItem.width * 0.35
                    to: control.contentItem.width
                    duration: 900
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
