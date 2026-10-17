import QtQuick

Item {
    id: root

    property real from: 0
    property real to: 100
    property real value: 0
    property real startAngle: -135
    property real endAngle: 135
    property real lineWidth: 10
    property color trackColor: Tokens.surfaceTertiary
    property color valueColor: Tokens.accent
    property color textColor: Tokens.textPrimary
    property bool showValue: true
    property string suffix: ""
    property int precision: 0

    readonly property real normalizedValue: to <= from ? 0
        : Math.max(0, Math.min(1, (value - from) / (to - from)))

    implicitWidth: 180
    implicitHeight: 180

    function requestPaint() { gaugeCanvas.requestPaint() }
    function radians(degrees) { return degrees * Math.PI / 180.0 }

    onNormalizedValueChanged: requestPaint()
    onStartAngleChanged: requestPaint()
    onEndAngleChanged: requestPaint()
    onLineWidthChanged: requestPaint()
    onTrackColorChanged: requestPaint()
    onValueColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    Canvas {
        id: gaugeCanvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            if (!ctx)
                return

            ctx.reset()
            const cx = width / 2
            const cy = height / 2
            const radius = Math.max(0, Math.min(width, height) / 2 - root.lineWidth)
            const start = root.radians(root.startAngle - 90)
            const finish = root.radians(root.endAngle - 90)
            const valueFinish = start + (finish - start) * root.normalizedValue

            ctx.lineCap = "round"
            ctx.lineWidth = root.lineWidth

            ctx.beginPath()
            ctx.arc(cx, cy, radius, start, finish, false)
            ctx.strokeStyle = root.trackColor
            ctx.stroke()

            if (root.normalizedValue > 0) {
                ctx.beginPath()
                ctx.arc(cx, cy, radius, start, valueFinish, false)
                ctx.strokeStyle = root.valueColor
                ctx.stroke()
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.showValue
            text: Number(root.value).toFixed(Math.max(0, root.precision)) + root.suffix
            color: root.textColor
            font.pixelSize: Math.max(18, Tokens.fontSizeBody + 8)
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.to > root.from
            text: Number(root.from).toFixed(0) + " – " + Number(root.to).toFixed(0)
            color: Tokens.textSecondary
            font.pixelSize: Tokens.fontSizeCaption
        }
    }
}
