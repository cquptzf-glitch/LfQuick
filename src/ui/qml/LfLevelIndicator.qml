import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property real from: 0
    property real to: 100
    property real value: 0
    property int orientation: Qt.Vertical
    property bool showText: true
    property color fillColor: Tokens.accent
    property color trackColor: Tokens.surfaceTertiary
    property string suffix: "%"

    readonly property real normalizedValue: to <= from ? 0
        : Math.max(0, Math.min(1, (value - from) / (to - from)))

    implicitWidth: orientation === Qt.Vertical ? 42 : 180
    implicitHeight: orientation === Qt.Vertical ? 180 : 30

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: root.trackColor
        border.color: Tokens.border
        border.width: 1
    }

    contentItem: Item {
        clip: true

        Rectangle {
            id: fill
            color: root.enabled ? root.fillColor : Tokens.textDisabled
            radius: Tokens.radiusSmall
            x: root.orientation === Qt.Horizontal ? 0 : 1
            y: root.orientation === Qt.Vertical ? parent.height - height : 1
            width: root.orientation === Qt.Horizontal
                 ? parent.width * root.normalizedValue
                 : Math.max(0, parent.width - 2)
            height: root.orientation === Qt.Vertical
                  ? parent.height * root.normalizedValue
                  : Math.max(0, parent.height - 2)

            Behavior on width {
                NumberAnimation { duration: Tokens.animationNormal; easing.type: Easing.OutCubic }
            }
            Behavior on height {
                NumberAnimation { duration: Tokens.animationNormal; easing.type: Easing.OutCubic }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: root.showText
            text: Math.round(root.normalizedValue * 100) + root.suffix
            color: root.normalizedValue > 0.55 ? Tokens.onAccent : Tokens.textPrimary
            font.pixelSize: Tokens.fontSizeCaption
            font.bold: true
        }
    }
}
