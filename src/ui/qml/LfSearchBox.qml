import QtQuick
import QtQuick.Controls as Controls

LfTextField {
    id: control

    property bool clearButtonVisible: true
    property string accessibilityName: placeholderText.length > 0 ? placeholderText : qsTr("Search")

    rightPadding: clearButton.visible ? Tokens.controlHeight : Tokens.controlPaddingHorizontal
    Accessible.name: accessibilityName
    Accessible.role: Accessible.EditableText

    Controls.ToolButton {
        id: clearButton
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(26, Tokens.controlHeight - 4)
        height: width
        visible: control.clearButtonVisible && control.enabled && control.text.length > 0
        focusPolicy: Qt.NoFocus
        text: "×"
        onClicked: {
            control.text = ""
            control.forceActiveFocus()
        }

        contentItem: Text {
            text: clearButton.text
            color: Tokens.textSecondary
            font.pixelSize: Tokens.fontSizeBody + 2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: Tokens.radiusSmall
            color: clearButton.down ? Tokens.controlFillPressed
                 : clearButton.hovered ? Tokens.controlFillHover
                 : "transparent"
        }
    }
}
