import QtQuick

Item {
    id: root

    property real level: 0
    property string label: ""
    property string detail: ""
    property color fillColor: Tokens.accent
    property color outlineColor: Tokens.borderStrong
    property color capColor: Tokens.surfaceTertiary
    property bool showPercent: true

    readonly property real normalizedLevel: Math.max(0, Math.min(1, level))

    implicitWidth: 100
    implicitHeight: 180

    Rectangle {
        id: cap
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.48
        height: Math.max(12, parent.height * 0.10)
        radius: Tokens.radiusSmall
        color: root.capColor
        border.color: root.outlineColor
    }

    Rectangle {
        id: body
        anchors.top: cap.bottom
        anchors.topMargin: -1
        anchors.bottom: labels.top
        anchors.bottomMargin: Tokens.spacingSmall
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: parent.width * 0.10
        anchors.rightMargin: parent.width * 0.10
        radius: Math.min(width, height) * 0.12
        color: "transparent"
        border.color: root.outlineColor
        border.width: 2
        clip: true

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 3
            height: Math.max(0, (parent.height - 6) * root.normalizedLevel)
            radius: Math.max(0, parent.radius - 3)
            color: root.enabled ? root.fillColor : Tokens.textDisabled

            Behavior on height {
                NumberAnimation { duration: Tokens.animationNormal; easing.type: Easing.OutCubic }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: root.showPercent
            text: Math.round(root.normalizedLevel * 100) + "%"
            color: root.normalizedLevel > 0.55 ? Tokens.onAccent : Tokens.textPrimary
            font.pixelSize: Tokens.fontSizeCaption
            font.bold: true
        }
    }

    Column {
        id: labels
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 1

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            visible: root.label.length > 0
            text: root.label
            color: Tokens.textPrimary
            font.pixelSize: Tokens.fontSizeBody
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            visible: root.detail.length > 0
            text: root.detail
            color: Tokens.textSecondary
            font.pixelSize: Tokens.fontSizeCaption
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }
}
