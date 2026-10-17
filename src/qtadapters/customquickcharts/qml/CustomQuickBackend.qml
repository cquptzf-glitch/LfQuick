import QtQuick

Item {
    id: root

    property var dataModel
    property int seriesType: 0 // ChartSeriesType::Line
    property real lineWidth: 2
    property real pointRadius: 3
    property real padding: 18
    property bool showGrid: true
    property int gridLines: 4
    property color backgroundColor: "transparent"
    property color seriesColor: systemPalette.highlight
    property color gridColor: Qt.rgba(systemPalette.text.r,
                                      systemPalette.text.g,
                                      systemPalette.text.b,
                                      0.16)

    readonly property var backendDescriptor: ({
        "id": "custom-quick",
        "name": "Custom Quick",
        "technology": "qt-quick",
        "priority": 60,
        "available": true,
        "seriesTypes": [0, 1, 2, 3],
        "metadata": { "renderer": "Canvas" }
    })
    property var resolvedBounds: ({ "valid": false })

    implicitWidth: 420
    implicitHeight: 260

    SystemPalette { id: systemPalette }

    function refreshBounds() {
        resolvedBounds = dataModel && dataModel.bounds
            ? dataModel.bounds() : ({ "valid": false })
        chartCanvas.requestPaint()
    }

    function safeSpan(minimum, maximum) {
        return Math.max(0.0000001, maximum - minimum)
    }

    function xFor(value) {
        if (!resolvedBounds.valid)
            return padding
        return padding + (Number(value) - resolvedBounds.minX)
               / safeSpan(resolvedBounds.minX, resolvedBounds.maxX)
               * Math.max(0, width - padding * 2)
    }

    function yFor(value) {
        if (!resolvedBounds.valid)
            return height - padding
        return height - padding - (Number(value) - resolvedBounds.minY)
               / safeSpan(resolvedBounds.minY, resolvedBounds.maxY)
               * Math.max(0, height - padding * 2)
    }

    function point(index) {
        return dataModel && dataModel.pointAt ? dataModel.pointAt(index) : ({})
    }

    onDataModelChanged: refreshBounds()
    onSeriesTypeChanged: chartCanvas.requestPaint()
    onSeriesColorChanged: chartCanvas.requestPaint()
    onGridColorChanged: chartCanvas.requestPaint()
    onBackgroundColorChanged: chartCanvas.requestPaint()
    onWidthChanged: chartCanvas.requestPaint()
    onHeightChanged: chartCanvas.requestPaint()

    Connections {
        target: root.dataModel
        function onBoundsChanged() { root.refreshBounds() }
        function onCountChanged() { root.refreshBounds() }
        function onDataChanged() { root.refreshBounds() }
        function onModelReset() { root.refreshBounds() }
        function onRowsInserted() { root.refreshBounds() }
        function onRowsRemoved() { root.refreshBounds() }
    }

    Canvas {
        id: chartCanvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            if (!ctx)
                return

            ctx.clearRect(0, 0, width, height)
            if (root.backgroundColor.a > 0) {
                ctx.fillStyle = root.backgroundColor
                ctx.fillRect(0, 0, width, height)
            }

            if (root.showGrid && root.gridLines > 0) {
                ctx.strokeStyle = root.gridColor
                ctx.lineWidth = 1
                for (let i = 0; i <= root.gridLines; ++i) {
                    const ratio = i / root.gridLines
                    const x = root.padding + ratio * Math.max(0, width - root.padding * 2)
                    const y = root.padding + ratio * Math.max(0, height - root.padding * 2)
                    ctx.beginPath(); ctx.moveTo(x, root.padding); ctx.lineTo(x, height - root.padding); ctx.stroke()
                    ctx.beginPath(); ctx.moveTo(root.padding, y); ctx.lineTo(width - root.padding, y); ctx.stroke()
                }
            }

            const count = root.dataModel && root.dataModel.count !== undefined
                        ? Number(root.dataModel.count) : 0
            if (count <= 0 || !root.resolvedBounds.valid)
                return

            ctx.strokeStyle = root.seriesColor
            ctx.fillStyle = root.seriesColor
            ctx.lineWidth = root.lineWidth

            if (root.seriesType === 0 || root.seriesType === 3) {
                ctx.beginPath()
                for (let index = 0; index < count; ++index) {
                    const p = root.point(index)
                    const x = root.xFor(p.x)
                    const y = root.yFor(p.y)
                    if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                }

                if (root.seriesType === 3 && count > 1) {
                    const last = root.point(count - 1)
                    const first = root.point(0)
                    ctx.lineTo(root.xFor(last.x), height - root.padding)
                    ctx.lineTo(root.xFor(first.x), height - root.padding)
                    ctx.closePath()
                    ctx.globalAlpha = 0.28
                    ctx.fill()
                    ctx.globalAlpha = 1.0
                } else {
                    ctx.stroke()
                }
            } else if (root.seriesType === 1) {
                for (let index = 0; index < count; ++index) {
                    const p = root.point(index)
                    ctx.beginPath()
                    ctx.arc(root.xFor(p.x), root.yFor(p.y), root.pointRadius, 0, Math.PI * 2)
                    ctx.fill()
                }
            } else if (root.seriesType === 2) {
                const available = Math.max(1, width - root.padding * 2)
                const barWidth = Math.max(1, available / Math.max(1, count) * 0.72)
                const zeroY = root.yFor(Math.max(root.resolvedBounds.minY,
                                                Math.min(root.resolvedBounds.maxY, 0)))
                for (let index = 0; index < count; ++index) {
                    const p = root.point(index)
                    const x = root.xFor(p.x) - barWidth / 2
                    const y = root.yFor(p.y)
                    ctx.fillRect(x, Math.min(y, zeroY), barWidth, Math.abs(zeroY - y))
                }
            }
        }
    }

    Component.onCompleted: refreshBounds()
}
