import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.DataView

LfPage {
    id: root
    title: qsTr("Results")

    DataFilterProxyModel {
        id: filteredModel
        sourceModel: resultModel
        filterText: filterField.text
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Results")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }

            Item { Layout.fillWidth: true }

            TextField {
                id: filterField
                Layout.preferredWidth: 280
                placeholderText: qsTr("Search result")
            }

            Button {
                text: qsTr("Sort Result")
                onClicked: resultModel.sortByKey("result", Qt.DescendingOrder)
            }
        }

        DataGrid {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: filteredModel
            columnWidths: [130, 110, 100, 100, 110, 150, 120]
            selectionMode: TableView.SingleSelection
        }
    }
}
