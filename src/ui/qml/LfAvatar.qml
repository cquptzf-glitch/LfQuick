import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property string name: ""
    property url source
    property int size: Theme.density === Theme.Touch ? 44 : 36
    property color backgroundColor: Tokens.accent
    property color foregroundColor: Tokens.onAccent

    readonly property string initials: {
        const trimmed = name.trim()
        if (trimmed.length === 0)
            return "?"
        const parts = trimmed.split(/\s+/)
        if (parts.length === 1)
            return parts[0].slice(0, 2).toUpperCase()
        return (parts[0].charAt(0) + parts[parts.length - 1].charAt(0)).toUpperCase()
    }

    implicitWidth: size
    implicitHeight: size
    padding: 0

    Accessible.name: name.length > 0 ? name : qsTr("Avatar")
    Accessible.role: Accessible.Graphic

    contentItem: Rectangle {
        radius: width / 2
        color: root.backgroundColor
        clip: true

        Image {
            id: avatarImage
            anchors.fill: parent
            visible: root.source.toString().length > 0
            source: root.source
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            cache: true
        }

        Text {
            anchors.centerIn: parent
            visible: !avatarImage.visible || avatarImage.status === Image.Error
            text: root.initials
            color: root.foregroundColor
            font.pixelSize: Math.max(Typography.caption, root.size * 0.36)
            font.weight: Typography.weightSemibold
        }
    }

    background: Item { }
}
