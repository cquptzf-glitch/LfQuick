import QtQuick
import QtCanvas2D

Item {
    id: root

    property var points: []
    property bool normalizedCoordinates: true
    property real padding: 0
    property real lineWidth: 2
    property color strokeColor: "#0067C0"
    property color backgroundColor: "transparent"
    property bool clearBeforePaint: true

    readonly property bool available: canvas.available
    readonly property alias context: canvas.context

    signal framePainted()

    function requestPaint() {
        canvas.requestPaint()
    }

    function pointX(point) {
        const x = Number(point && point.x !== undefined ? point.x : 0)
        return normalizedCoordinates
            ? padding + x * Math.max(0, canvas.width - padding * 2)
            : x
    }

    function pointY(point) {
        const y = Number(point && point.y !== undefined ? point.y : 0)
        return normalizedCoordinates
            ? canvas.height - padding - y * Math.max(0, canvas.height - padding * 2)
            : y
    }

    onPointsChanged: canvas.requestPaint()
    onStrokeColorChanged: canvas.requestPaint()
    onBackgroundColorChanged: canvas.requestPaint()
    onLineWidthChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Canvas2D {
        id: canvas
        anchors.fill: parent
        contextType: "2d"

        onAvailableChanged: {
            if (available)
                requestPaint()
        }

        onPaint: function(region) {
            const ctx = getContext("2d")
            if (!ctx)
                return

            if (root.clearBeforePaint)
                ctx.clearRect(0, 0, width, height)

            if (root.backgroundColor.a > 0) {
                ctx.fillStyle = root.backgroundColor
                ctx.fillRect(0, 0, width, height)
            }

            if (!root.points || root.points.length < 2)
                return

            ctx.beginPath()
            ctx.moveTo(root.pointX(root.points[0]), root.pointY(root.points[0]))
            for (let i = 1; i < root.points.length; ++i)
                ctx.lineTo(root.pointX(root.points[i]), root.pointY(root.points[i]))
            ctx.strokeStyle = root.strokeColor
            ctx.lineWidth = root.lineWidth
            ctx.stroke()
        }

        onPainted: root.framePainted()
    }
}
