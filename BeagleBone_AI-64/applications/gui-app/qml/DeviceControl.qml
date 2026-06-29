import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: surfaceColor
    radius: 10
    border.color: borderColor
    border.width: 1

    property string deviceId: ""
    property string deviceName: ""
    property string deviceType: ""
    property bool deviceEnabled: true
    property string deviceStatus: "Online"
    property bool isCalibrating: false
    property bool isTesting: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // Device info
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                width: 40
                height: 40
                radius: 20
                color: deviceEnabled ? successColor : errorColor

                Text {
                    anchors.centerIn: parent
                    text: deviceEnabled ? "✓" : "✗"
                    font.pixelSize: 20
                    color: "white"
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: root.deviceName || "Device"
                    font.pixelSize: 16
                    font.bold: true
                    color: textColor
                }

                Text {
                    text: root.deviceType + " • " + root.deviceStatus
                    font.pixelSize: 12
                    color: secondaryTextColor
                }
            }

            Text {
                text: root.deviceId
                font.pixelSize: 10
                color: secondaryTextColor
                visible: root.deviceId !== ""
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }

        // Device controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: root.deviceEnabled ? "Disable" : "Enable"
                Layout.fillWidth: true
                highlighted: root.deviceEnabled
                onClicked: {
                    if (root.deviceEnabled) {
                        deviceController.disableDevice(root.deviceId)
                    } else {
                        deviceController.enableDevice(root.deviceId)
                    }
                }
            }

            Button {
                text: "Reset"
                Layout.fillWidth: true
                enabled: root.deviceEnabled
                onClicked: {
                    deviceController.resetDevice(root.deviceId)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Calibrate"
                Layout.fillWidth: true
                enabled: root.deviceEnabled && !root.isCalibrating
                onClicked: {
                    root.isCalibrating = true
                    deviceController.calibrateDevice(root.deviceId, {})
                }
            }

            Button {
                text: "Self Test"
                Layout.fillWidth: true
                enabled: root.deviceEnabled && !root.isTesting
                onClicked: {
                    root.isTesting = true
                    deviceController.selfTestDevice(root.deviceId)
                }
            }
        }

        // Device properties
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            visible: false

            Text {
                text: "Manufacturer:"
                font.pixelSize: 11
                color: secondaryTextColor
            }
            Text {
                text: "Test Corp"
                font.pixelSize: 11
                color: textColor
            }

            Text {
                text: "Model:"
                font.pixelSize: 11
                color: secondaryTextColor
            }
            Text {
                text: "T-1000"
                font.pixelSize: 11
                color: textColor
            }

            Text {
                text: "Version:"
                font.pixelSize: 11
                color: secondaryTextColor
            }
            Text {
                text: "1.0.0"
                font.pixelSize: 11
                color: textColor
            }

            Text {
                text: "Serial:"
                font.pixelSize: 11
                color: secondaryTextColor
            }
            Text {
                text: "SN123456789"
                font.pixelSize: 11
                color: textColor
            }
        }

        // Status indicator
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: root.deviceStatus === "Online" ? successColor :
                       root.deviceStatus === "Warning" ? warningColor :
                       root.deviceStatus === "Error" ? errorColor :
                       secondaryTextColor
            }

            Text {
                text: "Last seen: " + new Date().toLocaleTimeString()
                font.pixelSize: 10
                color: secondaryTextColor
            }
        }
    }

    // Connections
    Connections {
        target: deviceController

        function onDeviceCalibrationComplete(deviceId, success) {
            if (deviceId === root.deviceId) {
                root.isCalibrating = false
                // Show result
            }
        }

        function onDeviceSelfTestComplete(deviceId, result) {
            if (deviceId === root.deviceId) {
                root.isTesting = false
                // Show result
            }
        }

        function onDeviceStatusChanged(deviceId, status) {
            if (deviceId === root.deviceId) {
                root.deviceStatus = status
            }
        }
    }
}
