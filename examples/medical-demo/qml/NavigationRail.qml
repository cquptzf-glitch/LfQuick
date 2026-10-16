
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    property int currentIndex: 0
    signal activated(int index)

    padding: 8

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Repeater {
            model: [qsTr("Dashboard"), qsTr("Samples"), qsTr("Results"), qsTr("Instrument"), qsTr("Alarms"), qsTr("Developer")]

            Button {
                required property string modelData
                required property int index
                Layout.fillWidth: true
                text: modelData
                checkable: true
                checked: index === root.currentIndex
                onClicked: root.activated(index)
            }
        }
        Item { Layout.fillHeight: true }
    }
}
