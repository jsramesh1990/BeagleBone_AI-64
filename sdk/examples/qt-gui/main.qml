// main.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSensors 5.15

ApplicationWindow {
    id: root
    width: 800
    height: 480
    visible: true
    title: "BBB AI-64 Dashboard"
    
    property bool darkTheme: false
    
    // Theme colors
    property color backgroundColor: darkTheme ? "#1a1a2e" : "#f5f5f5"
    property color surfaceColor: darkTheme ? "#16213e" : "#ffffff"
    property color primaryColor: "#00b4d8"
    property color textColor: darkTheme ? "#ffffff" : "#333333"
    
    // Sensor data
    property real temperature: 0
    property real humidity: 0
    property real pressure: 0
    
    // Timer for updating data
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            // Simulate sensor data
            temperature = 22.5 + Math.random() * 2
            humidity = 45 + Math.random() * 10
            pressure = 1013 + Math.random() * 5
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Text {
                text: "📊 BBB AI-64 Dashboard"
                font.pixelSize: 24
                font.bold: true
                color: textColor
                Layout.fillWidth: true
            }
            
            Switch {
                checked: root.darkTheme
                onCheckedChanged: {
                    root.darkTheme = checked
                }
                text: "🌙"
            }
        }
        
        // Sensor Grid
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 3
            rowSpacing: 15
            columnSpacing: 15
            
            // Temperature Sensor
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: surfaceColor
                radius: 10
                border.color: primaryColor
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 5
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Text {
                            text: "🌡️"
                            font.pixelSize: 24
                        }
                        
                        Text {
                            text: "Temperature"
                            font.pixelSize: 16
                            color: textColor
                            Layout.fillWidth: true
                        }
                    }
                    
                    Text {
                        text: temperature.toFixed(1) + "°C"
                        font.pixelSize: 32
                        font.bold: true
                        color: primaryColor
                        Layout.alignment: Qt.AlignCenter
                    }
                    
                    Text {
                        text: "Last update: " + new Date().toLocaleTimeString()
                        font.pixelSize: 10
                        color: "#999999"
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }
            
            // Humidity Sensor
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: surfaceColor
                radius: 10
                border.color: primaryColor
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 5
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Text {
                            text: "💧"
                            font.pixelSize: 24
                        }
                        
                        Text {
                            text: "Humidity"
                            font.pixelSize: 16
                            color: textColor
                            Layout.fillWidth: true
                        }
                    }
                    
                    Text {
                        text: humidity.toFixed(1) + "%"
                        font.pixelSize: 32
                        font.bold: true
                        color: primaryColor
                        Layout.alignment: Qt.AlignCenter
                    }
                }
            }
            
            // Pressure Sensor
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: surfaceColor
                radius: 10
                border.color: primaryColor
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 5
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Text {
                            text: "📊"
                            font.pixelSize: 24
                        }
                        
                        Text {
                            text: "Pressure"
                            font.pixelSize: 16
                            color: textColor
                            Layout.fillWidth: true
                        }
                    }
                    
                    Text {
                        text: pressure.toFixed(1) + " hPa"
                        font.pixelSize: 32
                        font.bold: true
                        color: primaryColor
                        Layout.alignment: Qt.AlignCenter
                    }
                }
            }
        }
        
        // Status Bar
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: surfaceColor
            radius: 5
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15
                
                Text {
                    text: "🟢 Connected"
                    font.pixelSize: 12
                    color: "#4ecdc4"
                }
                
                Text {
                    text: "📱 3 devices"
                    font.pixelSize: 12
                    color: "#999999"
                }
                
                Text {
                    text: "📈 " + new Date().toLocaleTimeString()
                    font.pixelSize: 12
                    color: "#999999"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
}
