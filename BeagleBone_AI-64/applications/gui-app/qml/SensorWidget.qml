import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: surfaceColor
    radius: 10
    border.color: primaryColor
    border.width: 1

    property string sensorType: "temperature"
    property string title: "Sensor"
    property string unit: ""
    property string icon: ""
    property variant value: 0
    property string status: "ok"
    property real minValue: 0
    property real maxValue: 100
    property real alertThreshold: 80
    property bool showGauge: true
    property bool showDetails: false

    property bool isAlert: {
        if (typeof value === 'number') {
            return value > alertThreshold
        }
        return false
    }

    // Status colors
    property color statusColor: {
        switch(status) {
            case "ok": return successColor
            case "warning": return warningColor
            case "error": return accentColor
            default: return secondaryTextColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Image {
                source: icon
                width: 32
                height: 32
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: title
                font.pixelSize: 16
                font.bold: true
                color: textColor
                Layout.fillWidth: true
            }

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: statusColor
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Value display
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Gauge (circular)
            CircularGauge {
                id: gauge
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height) * 0.8
                height: width
                visible: showGauge && typeof value === 'number'
                value: root.value
                minValue: root.minValue
                maxValue: root.maxValue
                unit: root.unit
                alertThreshold: root.alertThreshold
                color: isAlert ? accentColor : primaryColor
            }

            // Simple value display (for non-numeric)
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 5
                visible: !showGauge || typeof value !== 'number'

                Text {
                    text: root.value.toString()
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignCenter
                }

                Text {
                    text: root.unit
                    font.pixelSize: 14
                    color: secondaryTextColor
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }

        // Details (for GPS, IMU, etc.)
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            visible: showDetails

            Repeater {
                model: {
                    if (sensorType === "gps") {
                        return [
                            { label: "Latitude", value: dataManager.currentData.latitude || 0 },
                            { label: "Longitude", value: dataManager.currentData.longitude || 0 },
                            { label: "Altitude", value: dataManager.currentData.altitude || 0 },
                            { label: "Speed", value: dataManager.currentData.speed || 0 }
                        ]
                    } else if (sensorType === "imu") {
                        return [
                            { label: "Roll", value: dataManager.currentData.roll || 0 },
                            { label: "Pitch", value: dataManager.currentData.pitch || 0 },
                            { label: "Yaw", value: dataManager.currentData.yaw || 0 }
                        ]
                    }
                    return []
                }

                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: modelData.label + ":"
                        font.pixelSize: 12
                        color: secondaryTextColor
                        Layout.preferredWidth: 70
                    }

                    Text {
                        text: typeof modelData.value === 'number' ? 
                              modelData.value.toFixed(2) : modelData.value.toString()
                        font.pixelSize: 12
                        color: textColor
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Status text
        Text {
            text: status === "ok" ? "Online" : status.toUpperCase()
            font.pixelSize: 12
            color: statusColor
            Layout.alignment: Qt.AlignRight
        }
    }

    // Alert overlay
    Rectangle {
        anchors.fill: parent
        color: "red"
        opacity: 0.1
        visible: isAlert
        radius: 10
    }

    // Alert indicator
    Text {
        text: "⚠"
        font.pixelSize: 24
        color: accentColor
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        visible: isAlert
    }
}
