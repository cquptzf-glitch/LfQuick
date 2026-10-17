import QtQuick

Text {
    id: root

    property int codePoint: 0
    property string glyph: ""
    property string iconFontFamily: "Segoe Fluent Icons"
    property real iconSize: 18
    property string accessibleName: ""

    text: glyph.length > 0 ? glyph : (codePoint > 0 ? String.fromCodePoint(codePoint) : "")
    color: FluentTokens.text
    font.family: iconFontFamily
    font.pixelSize: iconSize
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    renderType: Text.NativeRendering
    Accessible.name: accessibleName
}
