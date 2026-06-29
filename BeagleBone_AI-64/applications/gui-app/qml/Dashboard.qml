import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true

    property var dashboard: Dashboard {
        id: dashboardInstance
    }

    // Grid layout for sensor widgets
    GridLayout {
        anchors.fill: parent
        anchors.margins: 20
        columns: Math.floor(width / 320)
        rowSpacing: 20
        columnSpacing: 20

        // Temperature sensor
        SensorWidget {
            id: tempWidget
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            sensorType: "temperature"
            title: "Temperature"
            unit: "°C"
            icon: "qrc:/icons/temperature.png"
            value: dataManager.currentData.temperature || 0
            status: dataManager.currentData.tempStatus || "ok"
            minValue: -10
            maxValue: 50
            alertThreshold: 35
        }

        // Humidity sensor
        SensorWidget {
            id: humidityWidget
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            sensorType: "humidity"
            title: "Humidity"
            unit: "%"
            icon: "qrc:/icons/humidity.png"
            value: dataManager.currentData.humidity || 0
            status: dataManager.currentData.humidityStatus || "ok"
            minValue: 0
            maxValue: 100
            alertThreshold: 80
        }

        // Pressure sensor
        SensorWidget {
            id: pressureWidget
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            sensorType: "pressure"
            title: "Pressure"
            unit: "hPa"
            icon: "qrc:/icons/pressure.png"
            value: dataManager.currentData.pressure || 0
            status: dataManager.currentData.pressureStatus || "ok"
            minValue: 900
            maxValue: 1100
            alertThreshold: 1050
        }

        // IMU sensor
        SensorWidget {
            id: imuWidget
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            sensorType: "imu"
            title: "IMU"
            unit: ""
            icon: "qrc:/icons/imu.png"
            value: "Roll: " + (dataManager.currentData.roll || 0).toFixed(1) + "°"
            status: dataManager.currentData.imuStatus || "ok"
            showGauge: false
        }

        // GPS sensor
        SensorWidget {
            id: gpsWidget
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            sensorType: "gps"
            title: "GPS"
            unit: ""
            icon: "qrc:/icons/gps.png"
            value: dataManager.currentData.gpsStatus || "No signal"
            status: dataManager.currentData.gpsStatus || "ok"
            showGauge: false
            showDetails: true
        }

        // Chart widget (spans 2 columns)
        ChartWidget {
            id: chartWidget
            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.preferredHeight: 300
            title: "Temperature History"
            chartModel: chartModel
        }
    }

    // Chart model for the chart widget
    ChartModel {
        id: chartModel
        deviceId: "temperature"
        dataKey: "value"
        maxPoints: 100
    }

    // Timer for updating data
    Timer {
        interval: dashboard.refreshInterval
        running: true
        repeat: true
        onTriggered: {
            dataManager.updateData()
            // Add data point to chart
            if (dataManager.currentData.temperature !== undefined) {
                chartModel.appendDataPoint(
                    new Date().getTime(),
                    dataManager.currentData.temperature,
                    new Date().toLocaleTimeString()
                )
            }
        }
    }

    // Connections to DataManager signals
    Connections {
        target: dataManager
        function onDataUpdated(data) {
            // Update UI with new data
            // The SensorWidgets will update automatically via property bindings
        }
        function onError(message) {
            alertBanner.showMessage(message, "error")
        }
    }

    // Connections to Dashboard
    Connections {
        target: dashboardInstance
        function onViewChanged(view) {
            // Handle view changes if needed
        }
    }
}
