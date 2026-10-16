
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    title: qsTr("Alarms")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Alarms"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("Persistent critical alert presentation and alarm history will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
