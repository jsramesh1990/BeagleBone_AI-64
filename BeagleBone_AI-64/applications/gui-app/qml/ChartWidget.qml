import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15

Rectangle {
    id: root
    color: surfaceColor
    radius: 10
    border.color: borderColor
    border.width: 1

    property string title: "Chart"
    property string unit: ""
    property var chartModel: null
    property string chartType: "line" // line, bar, scatter, area
    property bool showLegend: true
    property bool showGrid: true
    property bool showTooltip: true
    property color lineColor: primaryColor
    property color fillColor: Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.2)
    property color backgroundColor: "transparent"
    property real lineWidth: 2
    property real pointSize: 5

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // Header
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: root.title
                font.pixelSize: 16
                font.bold: true
                color: textColor
                Layout.fillWidth: true
            }

            Text {
                text: root.unit
                font.pixelSize: 12
                color: secondaryTextColor
                visible: root.unit !== ""
            }

            Button {
                text: "Clear"
                flat: true
                font.pixelSize: 12
                visible: root.chartModel && root.chartModel.count > 0
                onClicked: {
                    if (root.chartModel) {
                        root.chartModel.clear()
                    }
                }
            }
        }

        // Chart area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.backgroundColor
            clip: true

            // Chart view
            ChartView {
                id: chartView
                anchors.fill: parent
                theme: darkTheme ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
                antialiasing: true
                backgroundColor: "transparent"
                plotAreaColor: "transparent"
                legend.visible: root.showLegend
                legend.color: textColor
                legend.labelColor: textColor

                // Axes
                ValueAxis {
                    id: axisX
                    labelsColor: secondaryTextColor
                    gridVisible: root.showGrid
                    gridLineColor: borderColor
                    minorGridVisible: false
                    titleText: "Time"
                    titleColor: secondaryTextColor
                }

                ValueAxis {
                    id: axisY
                    labelsColor: secondaryTextColor
                    gridVisible: root.showGrid
                    gridLineColor: borderColor
                    minorGridVisible: false
                    titleText: root.unit
                    titleColor: secondaryTextColor
                }

                // Series based on chart type
                LineSeries {
                    id: lineSeries
                    visible: root.chartType === "line" || root.chartType === "area"
                    color: root.lineColor
                    width: root.lineWidth
                    pointsVisible: true
                    pointLabelsVisible: root.showTooltip
                    pointLabelsColor: textColor

                    // Fill for area chart
                    function fillArea() {
                        if (root.chartType === "area") {
                            color: root.fillColor
                        }
                    }
                }

                BarSeries {
                    id: barSeries
                    visible: root.chartType === "bar"
                    barWidth: 0.5
                    labelsVisible: root.showTooltip
                    labelsColor: textColor

                    BarSet {
                        id: barSet
                        label: "Data"
                        color: root.lineColor
                        borderColor: root.lineColor
                        values: []
                    }
                }

                ScatterSeries {
                    id: scatterSeries
                    visible: root.chartType === "scatter"
                    color: root.lineColor
                    markerSize: root.pointSize
                    markerShape: ScatterSeries.MarkerShapeCircle
                }

                // Update chart when model changes
                Connections {
                    target: root.chartModel
                    function onDataChanged() {
                        updateChart()
                    }
                    function onCountChanged() {
                        updateChart()
                    }
                }

                function updateChart() {
                    if (!root.chartModel) return

                    // Clear existing data
                    lineSeries.clear()
                    scatterSeries.clear()
                    barSet.values = []

                    // Get data from model
                    var data = root.chartModel.getData()
                    if (data.length === 0) return

                    // Process data for charts
                    var points = []
                    for (var i = 0; i < data.length; i++) {
                        var point = data[i]
                        points.push(Qt.point(point.x, point.y))
                    }

                    // Update line series
                    for (var j = 0; j < points.length; j++) {
                        lineSeries.append(points[j].x, points[j].y)
                        scatterSeries.append(points[j].x, points[j].y)
                    }

                    // Update bar series
                    var barValues = []
                    for (var k = 0; k < points.length; k++) {
                        barValues.push(points[k].y)
                    }
                    barSet.values = barValues

                    // Update axes ranges
                    if (root.chartModel.autoRange) {
                        axisX.min = root.chartModel.minValue || 0
                        axisX.max = root.chartModel.maxValue || 100
                        axisY.min = root.chartModel.minValue || 0
                        axisY.max = root.chartModel.maxValue || 100
                    }
                }

                Component.onCompleted: {
                    updateChart()
                }
            }

            // Empty state
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                visible: !root.chartModel || root.chartModel.count === 0

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "No Data"
                        font.pixelSize: 18
                        color: secondaryTextColor
                        Layout.alignment: Qt.AlignCenter
                    }

                    Text {
                        text: "Waiting for sensor data..."
                        font.pixelSize: 12
                        color: secondaryTextColor
                        Layout.alignment: Qt.AlignCenter
                    }
                }
            }
        }

        // Statistics
        RowLayout {
            Layout.fillWidth: true
            visible: root.chartModel && root.chartModel.count > 0
            spacing: 20

            Text {
                text: "Points: " + (root.chartModel ? root.chartModel.count : 0)
                font.pixelSize: 11
                color: secondaryTextColor
            }

            Text {
                text: "Min: " + (root.chartModel ? root.chartModel.minValue.toFixed(2) : 0) + " " + root.unit
                font.pixelSize: 11
                color: secondaryTextColor
            }

            Text {
                text: "Max: " + (root.chartModel ? root.chartModel.maxValue.toFixed(2) : 0) + " " + root.unit
                font.pixelSize: 11
                color: secondaryTextColor
            }

            Text {
                text: "Avg: " + (root.chartModel ? getAverage().toFixed(2) : 0) + " " + root.unit
                font.pixelSize: 11
                color: secondaryTextColor
            }

            function getAverage() {
                if (!root.chartModel || root.chartModel.count === 0) return 0
                var data = root.chartModel.getData()
                var sum = 0
                for (var i = 0; i < data.length; i++) {
                    sum += data[i].y
                }
                return sum / data.length
            }
        }
    }
}
