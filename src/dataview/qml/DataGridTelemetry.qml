import QtQuick

Item {
    id: root

    property Item target: null
    property bool enabled: true
    property real slowFrameThresholdMs: 33.3

    readonly property real fps: frameSampler.smoothFrameTime > 0
                                ? 1.0 / frameSampler.smoothFrameTime : 0
    readonly property real frameTimeMs: frameSampler.frameTime * 1000.0
    property real maxFrameTimeMs: 0
    property int sampleCount: 0
    property int slowFrameCount: 0
    property real firstVisibleMs: -1
    property double _startedAtMs: Date.now()

    visible: false
    width: 0
    height: 0

    function reset() {
        root.maxFrameTimeMs = 0
        root.sampleCount = 0
        root.slowFrameCount = 0
        root.firstVisibleMs = -1
        root._startedAtMs = Date.now()
        frameSampler.reset()
        root._probeFirstVisible()
    }

    function snapshot() {
        return {
            "fps": root.fps,
            "frameTimeMs": root.frameTimeMs,
            "maxFrameTimeMs": root.maxFrameTimeMs,
            "sampleCount": root.sampleCount,
            "slowFrameCount": root.slowFrameCount,
            "slowFrameThresholdMs": root.slowFrameThresholdMs,
            "firstVisibleMs": root.firstVisibleMs
        }
    }

    function _probeFirstVisible() {
        if (root.firstVisibleMs >= 0 || !root.target)
            return
        if (root.target.visible && root.target.width > 0 && root.target.height > 0)
            root.firstVisibleMs = Math.max(0, Date.now() - root._startedAtMs)
    }

    FrameAnimation {
        id: frameSampler
        running: root.enabled && root.target !== null && root.target.visible
        onTriggered: {
            const currentMs = frameTime * 1000.0
            root.sampleCount += 1
            root.maxFrameTimeMs = Math.max(root.maxFrameTimeMs, currentMs)
            if (currentMs > root.slowFrameThresholdMs)
                root.slowFrameCount += 1
            root._probeFirstVisible()
        }
    }

    Connections {
        target: root.target
        enabled: root.target !== null
        function onVisibleChanged() { root._probeFirstVisible() }
        function onWidthChanged() { root._probeFirstVisible() }
        function onHeightChanged() { root._probeFirstVisible() }
    }

    onTargetChanged: reset()
    Component.onCompleted: _probeFirstVisible()
}
