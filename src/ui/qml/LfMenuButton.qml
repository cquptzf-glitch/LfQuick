import QtQuick
import QtQuick.Controls as Controls

Controls.Button {
    id: control

    property var menu: null
    property bool openOnClick: true

    signal menuOpened()

    function openMenu() {
        if (!menu)
            return
        if (menu.parent !== control)
            menu.parent = control
        menu.x = 0
        menu.y = control.height + Units.xs
        menu.open()
        menuOpened()
    }

    implicitWidth: Math.max(88, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    contentItem: Row {
        anchors.centerIn: parent
        spacing: Units.sm

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
            font: control.font
            elide: Text.ElideRight
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "⌄"
            color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Typography.caption
        }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: control.down ? Tokens.controlFillPressed
             : control.hovered ? Tokens.controlFillHover
             : Tokens.controlFill
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.border
    }

    Connections {
        target: control
        function onClicked() {
            if (control.openOnClick)
                control.openMenu()
        }
    }
}
