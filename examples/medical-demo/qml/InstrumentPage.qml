
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    title: qsTr("Instrument")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Label { text: qsTr("Instrument"); font.pixelSize: 28; font.bold: true }
        Label { text: qsTr("Biochemistry / immunoassay module topology and live status will be implemented here."); wrapMode: Text.WordWrap; Layout.maximumWidth: 640 }
    }
}
