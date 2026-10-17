import QtQuick
import QtQuick.Controls as Controls

Controls.ProgressBar {
    id: root

    implicitWidth: 180
    implicitHeight: 6

    background: Rectangle {
        implicitWidth: root.implicitWidth
        implicitHeight: root.implicitHeight
        radius: height / 2
        color: FluentTokens.surfaceSecondary
    }

    contentItem: Item {
        implicitWidth: root.implicitWidth
        implicitHeight: root.implicitHeight
        clip: true

        Rectangle {
            width: root.indeterminate ? parent.width * 0.30 : parent.width * root.position
            height: parent.height
            radius: height / 2
            color: root.enabled ? FluentTokens.accent : FluentTokens.textSecondary

            SequentialAnimation on x {
                running: root.indeterminate && root.visible
                loops: Animation.Infinite
                NumberAnimation {
                    from: -parent.width * 0.30
                    to: parent.width
                    duration: 1100
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
