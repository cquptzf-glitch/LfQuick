import QtQuick

Item {
    id: root

    property var diagnosticsModel
    property bool expanded: true
    property int panelWidth: 480
    property int panelHeight: 520
    property int compactWidth: 190
    property int margin: 12
    property int maxVisibleMetrics: 8

    readonly property var metricEntries: {
        if (!root.diagnosticsModel)
            return []
        const map = root.diagnosticsModel.metrics
        const keys = Object.keys(map).sort()
        const entries = []
        for (let i = 0; i < Math.min(keys.length, root.maxVisibleMetrics); ++i)
            entries.push({ "key": keys[i], "value": map[keys[i]] })
        return entries
    }

    signal exportRequested()

    z: 100000
    visible: diagnosticsModel !== null && diagnosticsModel !== undefined

    Rectangle {
        id: panel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: root.margin
        anchors.rightMargin: root.margin
        width: root.expanded ? Math.min(root.panelWidth, root.width - root.margin * 2) : root.compactWidth
        height: root.expanded ? Math.min(root.panelHeight, root.height - root.margin * 2) : 42
        radius: 8
        color: DiagnosticsTokens.panel
        border.color: DiagnosticsTokens.panelBorder
        border.width: 1
        clip: true

        Behavior on width { NumberAnimation { duration: 100 } }
        Behavior on height { NumberAnimation { duration: 100 } }

        Rectangle {
            id: header
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 42
            color: DiagnosticsTokens.header

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: root.diagnosticsModel
                    ? "Diagnostics · " + root.diagnosticsModel.count
                    : "Diagnostics"
                color: DiagnosticsTokens.textStrong
                font.bold: true
                font.pixelSize: 14
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: root.expanded ? "−" : "+"
                color: DiagnosticsTokens.textSecondary
                font.pixelSize: 18
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.expanded = !root.expanded
            }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: header.bottom
            anchors.bottom: parent.bottom
            visible: root.expanded

            Text {
                id: droppedText
                anchors.left: parent.left
                anchors.right: clearButton.left
                anchors.top: parent.top
                anchors.leftMargin: 12
                anchors.topMargin: 10
                text: root.diagnosticsModel
                    ? "Events " + root.diagnosticsModel.count
                      + "   Dropped " + root.diagnosticsModel.droppedCount
                    : ""
                color: DiagnosticsTokens.textMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }

            Rectangle {
                id: clearButton
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 12
                anchors.topMargin: 7
                width: 58
                height: 26
                radius: 5
                color: clearMouse.containsMouse
                       ? DiagnosticsTokens.surfaceHover
                       : DiagnosticsTokens.surfaceRaised

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Clear")
                    color: DiagnosticsTokens.textButton
                    font.pixelSize: 11
                }

                MouseArea {
                    id: clearMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (root.diagnosticsModel)
                            root.diagnosticsModel.clearEvents()
                    }
                }
            }

            Flow {
                id: metricsFlow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: droppedText.bottom
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                anchors.topMargin: 10
                spacing: 6

                Repeater {
                    model: root.metricEntries

                    Rectangle {
                        required property var modelData
                        width: metricText.implicitWidth + 16
                        height: 24
                        radius: 4
                        color: DiagnosticsTokens.surface
                        border.color: DiagnosticsTokens.surfaceBorder

                        Text {
                            id: metricText
                            anchors.centerIn: parent
                            text: modelData.key + ": " + modelData.value
                            color: DiagnosticsTokens.textSecondary
                            font.pixelSize: 10
                        }
                    }
                }
            }

            Rectangle {
                id: divider
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: metricsFlow.bottom
                anchors.topMargin: 10
                height: 1
                color: DiagnosticsTokens.divider
            }

            ListView {
                id: eventList
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: divider.bottom
                anchors.bottom: footer.top
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.topMargin: 4
                clip: true
                spacing: 2
                model: root.diagnosticsModel

                onCountChanged: {
                    if (count > 0)
                        positionViewAtEnd()
                }

                delegate: Rectangle {
                    id: eventDelegate

                    required property int index
                    required property string timeText
                    required property string category
                    required property string name
                    required property string severity
                    required property string message

                    width: ListView.view.width
                    height: 42
                    radius: 4
                    color: index % 2 === 0
                           ? DiagnosticsTokens.rowEven
                           : DiagnosticsTokens.rowOdd

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 3
                        radius: 2
                        color: eventDelegate.severity === "error" || eventDelegate.severity === "critical"
                            ? DiagnosticsTokens.error
                            : eventDelegate.severity === "warning"
                                ? DiagnosticsTokens.warning
                                : DiagnosticsTokens.info
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 9
                        anchors.top: parent.top
                        anchors.topMargin: 4
                        text: eventDelegate.timeText + "  [" + eventDelegate.category + "] " + eventDelegate.name
                        color: DiagnosticsTokens.textPrimary
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        width: parent.width - 18
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 9
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 4
                        text: eventDelegate.message.length > 0 ? eventDelegate.message : eventDelegate.severity
                        color: DiagnosticsTokens.textDim
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }
            }

            Item {
                id: footer
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 34

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.diagnosticsModel && root.diagnosticsModel.lastExportError.length > 0
                        ? root.diagnosticsModel.lastExportError
                        : qsTr("Runtime dump is available through DiagnosticsModel.exportJson()")
                    color: root.diagnosticsModel && root.diagnosticsModel.lastExportError.length > 0
                        ? DiagnosticsTokens.errorText
                        : DiagnosticsTokens.textFaint
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    width: parent.width - 24
                }
            }
        }
    }
}
