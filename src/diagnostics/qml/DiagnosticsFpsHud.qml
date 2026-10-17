import QtQuick

Rectangle {
    id: root

    property var diagnosticsModel
    property string fpsMetric: "datagrid.fps"
    property string frameMetric: "datagrid.frameMs"
    property string maxFrameMetric: "datagrid.maxFrameMs"
    property string slowFrameMetric: "datagrid.slowFrames"
    property string title: qsTr("Performance")

    readonly property real fps: diagnosticsModel
        ? Number(diagnosticsModel.metric(fpsMetric) || 0) : 0
    readonly property real frameMs: diagnosticsModel
        ? Number(diagnosticsModel.metric(frameMetric) || 0) : 0
    readonly property real maxFrameMs: diagnosticsModel
        ? Number(diagnosticsModel.metric(maxFrameMetric) || 0) : 0
    readonly property int slowFrames: diagnosticsModel
        ? Number(diagnosticsModel.metric(slowFrameMetric) || 0) : 0

    color: "#e61c1c1f"
    border.color: "#707078"
    radius: 8
    implicitWidth: 240
    implicitHeight: 116

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 5

        Text {
            text: root.title
            color: "#f1f1f3"
            font.bold: true
            font.pixelSize: 12
        }

        Text {
            text: qsTr("FPS %1   Frame %2 ms")
                .arg(root.fps.toFixed(1))
                .arg(root.frameMs.toFixed(2))
            color: root.fps > 0 && root.fps < 45 ? "#ffb74d" : "#cfd0d4"
            font.pixelSize: 12
        }

        Text {
            text: qsTr("Max %1 ms   Slow %2")
                .arg(root.maxFrameMs.toFixed(2))
                .arg(root.slowFrames)
            color: root.slowFrames > 0 ? "#ffb74d" : "#9fa1a7"
            font.pixelSize: 11
        }

        Rectangle {
            width: parent.width
            height: 5
            radius: 2.5
            color: "#303037"

            Rectangle {
                width: parent.width * Math.max(0, Math.min(1, root.fps / 60.0))
                height: parent.height
                radius: parent.radius
                color: root.fps >= 55 ? "#66bb6a"
                     : root.fps >= 45 ? "#64b5f6"
                     : root.fps > 0 ? "#ffb74d" : "#777981"
            }
        }
    }
}
