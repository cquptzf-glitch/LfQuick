
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    title: qsTr("Samples")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Samples"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("DataGrid, scanner input and sample workflow PoC will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
