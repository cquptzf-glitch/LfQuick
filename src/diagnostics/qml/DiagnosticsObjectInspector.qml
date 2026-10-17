import QtQuick

Rectangle {
    id: root

    property var target
    property string filter: ""
    property int refreshInterval: 500
    property bool autoRefresh: true

    color: DiagnosticsTokens.panel
    border.color: DiagnosticsTokens.panelBorder
    radius: 8
    implicitWidth: 420
    implicitHeight: 420

    ObjectInspectorModel {
        id: inspectorModel
        target: root.target
        filter: root.filter
    }

    Timer {
        interval: Math.max(100, root.refreshInterval)
        running: root.visible && root.autoRefresh && root.target !== null
        repeat: true
        onTriggered: inspectorModel.refresh()
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        Text {
            width: parent.width
            text: root.target
                ? (root.target.objectName && root.target.objectName.length > 0
                   ? root.target.objectName : String(root.target))
                : qsTr("No object selected")
            color: DiagnosticsTokens.textStrong
            font.bold: true
            font.pixelSize: 13
            elide: Text.ElideRight
        }

        TextInput {
            id: filterEditor
            width: parent.width
            height: 30
            text: root.filter
            color: DiagnosticsTokens.textStrong
            selectionColor: DiagnosticsTokens.selection
            selectedTextColor: DiagnosticsTokens.onActive
            font.pixelSize: 12
            verticalAlignment: TextInput.AlignVCenter
            leftPadding: 8
            rightPadding: 8
            onTextChanged: root.filter = text

            Rectangle {
                anchors.fill: parent
                z: -1
                radius: 5
                color: DiagnosticsTokens.surface
                border.color: filterEditor.activeFocus
                              ? DiagnosticsTokens.info
                              : DiagnosticsTokens.surfaceHover
            }
        }

        ListView {
            width: parent.width
            height: parent.height - y
            clip: true
            spacing: 2
            model: inspectorModel

            delegate: Rectangle {
                required property int index
                required property string propertyName
                required property string propertyType
                required property string valueText
                required property bool writable
                required property bool dynamic

                width: ListView.view.width
                height: 44
                radius: 4
                color: index % 2 === 0
                       ? DiagnosticsTokens.rowEven
                       : DiagnosticsTokens.rowOdd

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    width: parent.width * 0.43
                    text: propertyName + (dynamic ? " *" : "")
                    color: writable
                           ? DiagnosticsTokens.textPrimary
                           : DiagnosticsTokens.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    width: parent.width * 0.42
                    horizontalAlignment: Text.AlignRight
                    text: propertyType
                    color: DiagnosticsTokens.textFaint
                    font.pixelSize: 9
                    elide: Text.ElideLeft
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                    text: valueText
                    color: DiagnosticsTokens.textDim
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }
    }
}
