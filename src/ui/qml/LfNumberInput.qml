import QtQuick
import QtQuick.Controls as Controls

Controls.SpinBox {
    id: control

    editable: true
    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlHeight
    font.pixelSize: Tokens.fontSizeBody

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
        selectionColor: Tokens.selection
        selectedTextColor: Tokens.textPrimary
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        selectByMouse: true
    }

    up.indicator: Rectangle {
        x: control.width - width
        y: 0
        implicitWidth: Tokens.controlHeight / 2
        implicitHeight: control.height / 2
        color: control.up.pressed ? Tokens.controlFillPressed
             : control.up.hovered ? Tokens.controlFillHover
             : "transparent"
        Text {
            anchors.centerIn: parent
            text: "+"
            color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Tokens.fontSizeCaption
        }
    }

    down.indicator: Rectangle {
        x: control.width - width
        y: control.height / 2
        implicitWidth: Tokens.controlHeight / 2
        implicitHeight: control.height / 2
        color: control.down.pressed ? Tokens.controlFillPressed
             : control.down.hovered ? Tokens.controlFillHover
             : "transparent"
        Text {
            anchors.centerIn: parent
            text: "−"
            color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Tokens.fontSizeCaption
        }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: control.enabled ? Tokens.controlFill : Tokens.controlFillDisabled
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.border
    }
}
