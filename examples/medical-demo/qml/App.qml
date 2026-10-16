
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1440
    height: 900
    visible: true
    title: qsTr("LfQuick Medical Demo")

    property int currentPage: 0

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            Label { text: "LfQuick Medical Demo"; font.bold: true }
            Item { Layout.fillWidth: true }
            Label { text: qsTr("Instrument: Ready") }
            Label { text: qsTr("Scenario: Normal") }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationRail {
            Layout.preferredWidth: 190
            Layout.fillHeight: true
            currentIndex: window.currentPage
            onActivated: index => window.currentPage = index
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: window.currentPage

            DashboardPage {}
            SamplesPage {}
            ResultsPage {}
            InstrumentPage {}
            AlarmsPage {}
            DeveloperPage {}
        }
    }
}
