import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: surfaceColor
    border.top.color: borderColor
    border.top.width: 1

    property int deviceCount: deviceController.deviceCount
    property bool isConnected: sensorClient.connected
    property string currentTime: ""

    RowLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 15

        // Connection status
        RowLayout {
            spacing: 5

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.isConnected ? successColor : errorColor
            }

            Text {
                text: root.isConnected ? "Connected" : "Disconnected"
                font.pixelSize: 11
                color: root.isConnected ? successColor : errorColor
            }
        }

        // Device count
        RowLayout {
            spacing: 5

            Text {
                text: "📱"
                font.pixelSize: 12
            }

            Text {
                text: root.deviceCount + " devices"
                font.pixelSize: 11
                color: secondaryTextColor
            }
        }

        // Data points
        RowLayout {
            spacing: 5

            Text {
                text: "📊"
                font.pixelSize: 12
            }

            Text {
                text: dataManager.dataPoints + " data points"
                font.pixelSize: 11
                color: secondaryTextColor
            }
        }

        // Last update
        Text {
            text: "Last update: " + new Date().toLocaleTimeString()
            font.pixelSize: 11
            color: secondaryTextColor
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
        }

        // Update status
        BusyIndicator {
            running: dataManager.isUpdating
            width: 16
            height: 16
            visible: dataManager.isUpdating
        }

        // Status messages
        Text {
            id: statusMessage
            text: ""
            font.pixelSize: 11
            color: secondaryTextColor
            visible: text !== ""
        }
    }

    // Timer for updating time
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            root.currentTime = new Date().toLocaleTimeString()
        }
    }

    // Function to show status message
    function showMessage(message, type) {
        statusMessage.text = message
        statusMessage.color = type === "error" ? errorColor :
                             type === "warning" ? warningColor :
                             secondaryTextColor
        showTimer.start()
    }

    Timer {
        id: showTimer
        interval: 5000
        onTriggered: {
            statusMessage.text = ""
        }
    }
}
