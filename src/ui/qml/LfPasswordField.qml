import QtQuick
import QtQuick.Controls as Controls

LfTextField {
    id: control

    property bool passwordVisible: false
    property bool revealButtonVisible: true

    echoMode: passwordVisible ? TextInput.Normal : TextInput.Password
    inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
    rightPadding: revealButton.visible ? Tokens.controlHeight : Tokens.controlPaddingHorizontal
    Accessible.name: placeholderText.length > 0 ? placeholderText : qsTr("Password")

    Controls.ToolButton {
        id: revealButton
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(26, Tokens.controlHeight - 4)
        height: width
        visible: control.revealButtonVisible && control.enabled && control.text.length > 0
        focusPolicy: Qt.NoFocus
        text: control.passwordVisible ? qsTr("Hide") : qsTr("Show")
        onClicked: {
            control.passwordVisible = !control.passwordVisible
            control.forceActiveFocus()
        }

        contentItem: Text {
            text: revealButton.text
            color: Tokens.textSecondary
            font.pixelSize: Tokens.fontSizeCaption
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: Tokens.radiusSmall
            color: revealButton.down ? Tokens.controlFillPressed
                 : revealButton.hovered ? Tokens.controlFillHover
                 : "transparent"
        }
    }
}
