import QtQuick
import QtQuick.Controls as Controls

Controls.TextField {
    id: root

    implicitHeight: BrandTokens.controlHeight
    leftPadding: 14
    rightPadding: 14
    topPadding: 0
    bottomPadding: 0
    selectByMouse: true
    color: root.enabled ? BrandTokens.text : BrandTokens.textSecondary
    placeholderTextColor: BrandTokens.textSecondary
    selectionColor: BrandTokens.accent
    selectedTextColor: BrandTokens.onAccent
    font.pixelSize: BrandTokens.fontSize

    background: Rectangle {
        radius: BrandTokens.radius
        color: root.enabled ? BrandTokens.control : BrandTokens.surfaceRaised
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? BrandTokens.accent : BrandTokens.border
    }
}
