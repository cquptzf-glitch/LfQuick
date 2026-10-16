import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick
import LfQuick.Charts

LfPage {
    id: root
    title: qsTr("Charts")

    property string seriesMode: "line"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: qsTr("Qt Graphs adapter validation")
                    font.pixelSize: 28
                    font.bold: true
                    color: Tokens.textPrimary
                }

                Label {
                    text: qsTr("Synthetic trend data only. This page validates the generic Charts contract and renderer adapter; it contains no diagnostic, QC, or calibration algorithm.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    color: Tokens.textSecondary
                }
            }

            Button {
                text: qsTr("Line")
                checkable: true
                checked: root.seriesMode === "line"
                onClicked: root.seriesMode = "line"
            }

            Button {
                text: qsTr("Scatter")
                checkable: true
                checked: root.seriesMode === "scatter"
                onClicked: root.seriesMode = "scatter"
            }

            Button {
                text: qsTr("Append point")
                onClicked: {
                    const x = syntheticChartModel.count
                    const y = 50 + Math.sin(x * 0.12) * 8 + Math.cos(x * 0.035) * 3
                    syntheticChartModel.appendPoint(x, y, { "source": "interactive-demo" })
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Tokens.radiusMedium
            color: Tokens.surfaceSecondary
            border.color: Tokens.border
            clip: true

            ChartView {
                anchors.fill: parent
                anchors.margins: 12
                dataModel: syntheticChartModel
                seriesType: root.seriesMode
                seriesName: qsTr("Synthetic signal")
                xTitle: qsTr("Sequence")
                yTitle: qsTr("Synthetic value")
                yLabelFormat: "%.1f"
            }
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Points: %1 | Renderer: Qt Graphs | Mode: %2")
                .arg(syntheticChartModel.count)
                .arg(root.seriesMode)
            color: Tokens.textSecondary
        }
    }
}
