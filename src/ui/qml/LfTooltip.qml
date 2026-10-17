import QtQuick
import QtQuick.Controls as Controls

Controls.ToolTip {
    id: root

    delay: 600
    timeout: 5000
    padding: Tokens.spacingSmall

    contentItem: Text {
        text: root.text
        color: Tokens.textPrimary
        font.pixelSize: Tokens.fontSizeCaption
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: Tokens.surface
        border.color: Tokens.borderStrong
        border.width: 1
    }
}
