import QtQuick
import QtGraphs

Item {
    id: root

    property var dataModel
    property string seriesType: "line"
    property string seriesName: "Series"
    property string xTitle: ""
    property string yTitle: ""
    property string xLabelFormat: ""
    property string yLabelFormat: ""
    property real rangePaddingRatio: 0.05

    readonly property bool supportedSeriesType: seriesType === "line" || seriesType === "scatter"
    property var resolvedBounds: ({ "valid": false })

    function refreshBounds() {
        resolvedBounds = dataModel ? dataModel.bounds() : ({ "valid": false })
    }

    function paddedMinimum(minimum, maximum) {
        if (maximum <= minimum)
            return minimum - 1.0
        return minimum - (maximum - minimum) * rangePaddingRatio
    }

    function paddedMaximum(minimum, maximum) {
        if (maximum <= minimum)
            return maximum + 1.0
        return maximum + (maximum - minimum) * rangePaddingRatio
    }

    GraphsView {
        id: graph
        anchors.fill: parent

        axisX: ValueAxis {
            titleText: root.xTitle
            labelFormat: root.xLabelFormat
            min: root.resolvedBounds.valid
                ? root.paddedMinimum(root.resolvedBounds.minX, root.resolvedBounds.maxX)
                : 0
            max: root.resolvedBounds.valid
                ? root.paddedMaximum(root.resolvedBounds.minX, root.resolvedBounds.maxX)
                : 1
        }

        axisY: ValueAxis {
            titleText: root.yTitle
            labelFormat: root.yLabelFormat
            min: root.resolvedBounds.valid
                ? root.paddedMinimum(root.resolvedBounds.minY, root.resolvedBounds.maxY)
                : 0
            max: root.resolvedBounds.valid
                ? root.paddedMaximum(root.resolvedBounds.minY, root.resolvedBounds.maxY)
                : 1
        }

        LineSeries {
            id: lineSeries
            name: root.seriesName
            visible: root.seriesType === "line"

            XYModelMapper {
                model: root.dataModel
                series: lineSeries
                orientation: Qt.Vertical
                xSection: 0
                ySection: 1
            }
        }

        ScatterSeries {
            id: scatterSeries
            name: root.seriesName
            visible: root.seriesType === "scatter"

            XYModelMapper {
                model: root.dataModel
                series: scatterSeries
                orientation: Qt.Vertical
                xSection: 0
                ySection: 1
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: !root.supportedSeriesType
        text: qsTr("Unsupported series type: %1").arg(root.seriesType)
    }

    Connections {
        target: root.dataModel
        function onBoundsChanged() { root.refreshBounds() }
    }

    onDataModelChanged: refreshBounds()
    Component.onCompleted: refreshBounds()
}
