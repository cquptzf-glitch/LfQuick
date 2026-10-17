import QtQuick

Item {
    id: root

    property url source: ""
    property string glyph: ""
    property string fallbackText: ""
    property color color: Tokens.textPrimary
    property real iconSize: Units.iconSize
    property int fillMode: Image.PreserveAspectFit
    property bool smooth: true
    property string accessibleName: ""

    readonly property bool imageReady: image.status === Image.Ready
    readonly property bool imageFailed: image.status === Image.Error

    implicitWidth: iconSize
    implicitHeight: iconSize
    Accessible.name: accessibleName

    Image {
        id: image
        anchors.centerIn: parent
        width: root.iconSize
        height: root.iconSize
        source: root.source
        fillMode: root.fillMode
        smooth: root.smooth
        visible: root.source.toString().length > 0 && status !== Image.Error
    }

    Text {
        anchors.centerIn: parent
        visible: !image.visible
        text: root.glyph.length > 0 ? root.glyph : root.fallbackText
        color: root.color
        font.pixelSize: root.iconSize
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        renderType: Text.NativeRendering
    }
}
