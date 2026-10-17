import QtQuick

Rectangle {
    id: root

    property var runtime

    color: DiagnosticsTokens.panel
    border.color: DiagnosticsTokens.panelBorder
    radius: 8
    implicitWidth: 360
    implicitHeight: 220

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 7

        Text {
            text: qsTr("Route Inspector")
            color: DiagnosticsTokens.textStrong
            font.bold: true
            font.pixelSize: 12
        }

        Text {
            width: parent.width
            text: root.runtime
                ? qsTr("Current: %1").arg(root.runtime.currentRoute || "-")
                : qsTr("No runtime")
            color: DiagnosticsTokens.textPrimary
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
            color: DiagnosticsTokens.textSecondary
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
            color: DiagnosticsTokens.textDim
            font.pixelSize: 10
        }

        Row {
            spacing: 8
            visible: root.runtime !== null

            Rectangle {
                width: 68
                height: 26
                radius: 4
                color: backMouse.containsMouse
                       ? DiagnosticsTokens.surfaceHover
                       : DiagnosticsTokens.surface
                opacity: root.runtime && root.runtime.canGoBack ? 1.0 : 0.4
                Text {
                    anchors.centerIn: parent
                    text: qsTr("Back")
                    color: DiagnosticsTokens.textPrimary
                    font.pixelSize: 10
                }
                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: root.runtime && root.runtime.canGoBack
                    onClicked: root.runtime.back()
                }
            }

            Rectangle {
                width: 68
                height: 26
                radius: 4
                color: forwardMouse.containsMouse
                       ? DiagnosticsTokens.surfaceHover
                       : DiagnosticsTokens.surface
                opacity: root.runtime && root.runtime.canGoForward ? 1.0 : 0.4
                Text {
                    anchors.centerIn: parent
                    text: qsTr("Forward")
                    color: DiagnosticsTokens.textPrimary
                    font.pixelSize: 10
                }
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
