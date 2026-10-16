
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    title: qsTr("Results")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Results"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("High-density result DataGrid and review workflow will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
