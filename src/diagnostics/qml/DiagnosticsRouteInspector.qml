import QtQuick

Rectangle {
    id: root

    property var runtime

    color: "#e61c1c1f"
    border.color: "#707078"
    radius: 8
    implicitWidth: 360
    implicitHeight: 220

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 7

        Text {
            text: qsTr("Route Inspector")
            color: "#f1f1f3"
            font.bold: true
            font.pixelSize: 12
        }

        Text {
            width: parent.width
            text: root.runtime
                ? qsTr("Current: %1").arg(root.runtime.currentRoute || "-")
                : qsTr("No runtime")
            color: "#e1e1e4"
            font.pixelSize: 11
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.runtime
                ? qsTr("Back %1 · Forward %2 · Depth %3 · Forward depth %4")
                    .arg(root.runtime.canGoBack ? "yes" : "no")
                    .arg(root.runtime.canGoForward ? "yes" : "no")
                    .arg(root.runtime.depth)
                    .arg(root.runtime.forwardDepth)
                : ""
            color: "#cfd0d4"
            font.pixelSize: 10
        }

        Text {
            width: parent.width
            text: root.runtime
                ? qsTr("Singletons %1 · LRU weight %2 / %3")
                    .arg(root.runtime.singletonCount)
                    .arg(root.runtime.retainedLruWeight)
                    .arg(root.runtime.maxRetainedLruWeight)
                : ""
            color: "#9fa1a7"
            font.pixelSize: 10
        }

        Row {
            spacing: 8
            visible: root.runtime !== null

            Rectangle {
                width: 68; height: 26; radius: 4
                color: backMouse.containsMouse ? "#4b4b52" : "#303037"
                opacity: root.runtime && root.runtime.canGoBack ? 1.0 : 0.4
                Text { anchors.centerIn: parent; text: qsTr("Back"); color: "#e1e1e4"; font.pixelSize: 10 }
                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: root.runtime && root.runtime.canGoBack
                    onClicked: root.runtime.back()
                }
            }

            Rectangle {
                width: 68; height: 26; radius: 4
                color: forwardMouse.containsMouse ? "#4b4b52" : "#303037"
                opacity: root.runtime && root.runtime.canGoForward ? 1.0 : 0.4
                Text { anchors.centerIn: parent; text: qsTr("Forward"); color: "#e1e1e4"; font.pixelSize: 10 }
                MouseArea {
                    id: forwardMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: root.runtime && root.runtime.canGoForward
                    onClicked: root.runtime.forward()
                }
            }
        }
    }
}
