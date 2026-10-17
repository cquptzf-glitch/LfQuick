import QtQuick
import QtQuick.Controls as Controls

Controls.Popup {
    id: root

    padding: Tokens.spacingMedium
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Tokens.surface
        border.color: Tokens.border
        border.width: 1
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: Tokens.animationFast
        }
        NumberAnimation {
            property: "scale"
            from: 0.98
            to: 1
            duration: Tokens.animationFast
            easing.type: Easing.OutCubic
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: Tokens.animationFast
        }
    }
}
