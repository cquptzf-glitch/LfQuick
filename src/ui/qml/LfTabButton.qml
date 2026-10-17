import QtQuick
import QtQuick.Controls as Controls

Controls.TabButton {
    id: control

    implicitHeight: Tokens.controlHeight + Units.xs
    implicitWidth: Math.max(88, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    contentItem: Text {
        text: control.text
        color: !control.enabled ? Tokens.textDisabled
             : control.checked ? Tokens.accent
             : Tokens.textPrimary
        font: control.font
        font.weight: control.checked ? Typography.weightSemibold : Typography.weightRegular
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            radius: Tokens.radiusSmall
            color: control.down ? Tokens.controlFillPressed
                 : control.hovered ? Tokens.controlFillHover
                 : "transparent"
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: Units.sm
            anchors.rightMargin: Units.sm
            height: control.checked ? 3 : 0
            radius: height / 2
            color: Tokens.accent

            Behavior on height {
                NumberAnimation { duration: Tokens.animationFast }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: Tokens.radiusSmall
            color: "transparent"
            border.width: control.activeFocus ? 2 : 0
            border.color: Tokens.focusRing
        }
    }
}
