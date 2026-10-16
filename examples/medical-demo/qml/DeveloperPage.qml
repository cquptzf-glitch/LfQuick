
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    title: qsTr("Developer")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Developer"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("Focus, overlay, DataGrid and chart stress tools will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
