import QtQuick

Rectangle {
    id: root

    property var focusNavigator
    property int refreshInterval: 300
    property var focusEntries: []

    color: DiagnosticsTokens.panel
    border.color: DiagnosticsTokens.panelBorder
    radius: 8
    implicitWidth: 360
    implicitHeight: 360

    function refresh() {
        focusEntries = focusNavigator && focusNavigator.dumpOrder
            ? focusNavigator.dumpOrder() : []
    }

    Timer {
        interval: Math.max(100, root.refreshInterval)
        running: root.visible && root.focusNavigator !== null
        repeat: true
        triggeredOnStart: true
        onTriggered: root.refresh()
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        Text {
            width: parent.width
            text: root.focusNavigator
                ? qsTr("Focus Inspector · %1 items").arg(root.focusEntries.length)
                : qsTr("Focus Inspector · no navigator")
            color: DiagnosticsTokens.textStrong
            font.bold: true
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.focusNavigator && root.focusNavigator.currentFocus
                ? qsTr("Current: %1").arg(root.focusNavigator.currentFocus.objectName
                    || String(root.focusNavigator.currentFocus))
                : qsTr("Current: none")
            color: DiagnosticsTokens.textDim
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        ListView {
            width: parent.width
            height: parent.height - y
            clip: true
            spacing: 2
            model: root.focusEntries

            delegate: Rectangle {
                required property int index
                required property var modelData
                width: ListView.view.width
                height: 42
                radius: 4
                color: modelData.active
                       ? DiagnosticsTokens.rowActive
                       : (index % 2 ? DiagnosticsTokens.rowOdd : DiagnosticsTokens.rowEven)

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    width: parent.width - 16
                    text: (modelData.index + 1) + ". " + modelData.name
                          + (modelData.focusOrder ? "  order=" + modelData.focusOrder : "")
                    color: modelData.active
                           ? DiagnosticsTokens.onActive
                           : DiagnosticsTokens.textPrimary
                    font.pixelSize: 10
                    font.bold: modelData.active
                    elide: Text.ElideRight
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                    text: "x=" + Math.round(modelData.x)
                          + " y=" + Math.round(modelData.y)
                          + "  " + Math.round(modelData.width)
                          + "×" + Math.round(modelData.height)
                    color: DiagnosticsTokens.textGeometry
                    font.pixelSize: 9
                }
            }
        }
    }
}
