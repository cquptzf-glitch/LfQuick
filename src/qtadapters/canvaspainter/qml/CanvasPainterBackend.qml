import QtQuick

Item {
    id: root

    property alias points: surface.points
    property alias normalizedCoordinates: surface.normalizedCoordinates
    property alias padding: surface.padding
    property alias lineWidth: surface.lineWidth
    property alias strokeColor: surface.strokeColor
    property alias backgroundColor: surface.backgroundColor
    property alias clearBeforePaint: surface.clearBeforePaint
    readonly property bool available: surface.available
    readonly property var backendDescriptor: ({
        "id": "canvas-painter",
        "name": "Canvas Painter",
        "technology": "qt-canvas2d",
        "priority": 80,
        "available": root.available,
        "seriesTypes": [0, 1],
        "metadata": { "qtMinimum": "6.12", "technologyPreview": true }
    })

    signal framePainted()

    function requestPaint() {
        surface.requestPaint()
    }

    Canvas2DSurface {
        id: surface
        anchors.fill: parent
        onFramePainted: root.framePainted()
    }
}
