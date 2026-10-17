import QtQuick
import QtQuick.Controls as Controls

Controls.TextField {
    id: root

    implicitHeight: FluentTokens.controlHeight
    leftPadding: FluentTokens.padding
    rightPadding: FluentTokens.padding
    topPadding: 0
    bottomPadding: 0
    selectByMouse: true
    color: root.enabled ? FluentTokens.text : FluentTokens.textSecondary
    placeholderTextColor: FluentTokens.textSecondary
    selectionColor: FluentTokens.accent
    selectedTextColor: FluentTokens.onAccent
    font.pixelSize: FluentTokens.fontSize

    background: Rectangle {
        radius: FluentTokens.radius
        color: root.enabled ? FluentTokens.fill : FluentTokens.disabled
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent : FluentTokens.border
    }
}
