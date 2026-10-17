import QtQuick
import QtQuick.Controls as Controls

Controls.TextField {
    id: control

    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: 0
    bottomPadding: 0
    selectByMouse: true
    color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
    placeholderTextColor: Tokens.textSecondary
    selectionColor: Tokens.selection
    selectedTextColor: Tokens.textPrimary
    font.pixelSize: Tokens.fontSizeBody

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: control.enabled ? Tokens.controlFill : Tokens.controlFillDisabled
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing
                    : control.hovered ? Tokens.borderStrong
                    : Tokens.border

        Behavior on border.color {
            ColorAnimation { duration: Tokens.animationFast }
        }
    }
}
