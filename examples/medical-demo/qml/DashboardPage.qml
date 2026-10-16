
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    title: qsTr("Dashboard")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Dashboard"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("Instrument overview, throughput, QC and reagent status will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
