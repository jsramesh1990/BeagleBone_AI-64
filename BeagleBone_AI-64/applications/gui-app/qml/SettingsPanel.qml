import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: surfaceColor
    radius: 15
    border.color: borderColor
    border.width: 1

    property bool darkMode: settingsManager.darkTheme

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Header
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "⚙️ Settings"
                font.pixelSize: 20
                font.bold: true
                color: textColor
                Layout.fillWidth: true
            }

            Button {
                text: "✕"
                flat: true
                onClicked: root.visible = false
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }

        // Settings groups
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: parent.width
                spacing: 15

                // Theme settings
                GroupBox {
                    title: "Theme"
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: 10

                        Text {
                            text: "Dark Mode"
                            color: textColor
                            Layout.fillWidth: true
                        }

                        Switch {
                            id: themeSwitch
                            checked: settingsManager.darkTheme
                            onCheckedChanged: {
                                settingsManager.darkTheme = checked
                                dashboard.toggleTheme()
                            }
                        }
                    }
                }

                // Display settings
                GroupBox {
                    title: "Display"
                    Layout.fillWidth: true

                    ColumnLayout {
                        spacing: 10

                        RowLayout {
                            spacing: 10

                            Text {
                                text: "Refresh Interval"
                                color: textColor
                                Layout.fillWidth: true
                            }

                            SpinBox {
                                id: refreshSpin
                                from: 100
                                to: 10000
                                stepSize: 100
                                value: settingsManager.refreshInterval
                                editable: true
                                onValueChanged: {
                                    settingsManager.refreshInterval = value
                                }
                            }

                            Text {
                                text: "ms"
                                color: secondaryTextColor
                            }
                        }

                        RowLayout {
                            spacing: 10

                            Text {
                                text: "Auto Start"
                                color: textColor
                                Layout.fillWidth: true
                            }

                            Switch {
                                id: autoStartSwitch
                                checked: settingsManager.autoStart
                                onCheckedChanged: {
                                    settingsManager.autoStart = checked
                                }
                            }
                        }
                    }
                }

                // Notification settings
                GroupBox {
                    title: "Notifications"
                    Layout.fillWidth: true

                    ColumnLayout {
                        spacing: 10

                        RowLayout {
                            spacing: 10

                            Text {
                                text: "Show Notifications"
                                color: textColor
                                Layout.fillWidth: true
                            }

                            Switch {
                                id: notificationSwitch
                                checked: settingsManager.showNotifications
                                onCheckedChanged: {
                                    settingsManager.showNotifications = checked
                                }
                            }
                        }

                        RowLayout {
                            spacing: 10

                            Text {
                                text: "Sound Enabled"
                                color: textColor
                                Layout.fillWidth: true
                            }

                            Switch {
                                id: soundSwitch
                                checked: settingsManager.soundEnabled
                                onCheckedChanged: {
                                    settingsManager.soundEnabled = checked
                                }
                            }
                        }
                    }
                }

                // Server settings
                GroupBox {
                    title: "Server Connection"
                    Layout.fillWidth: true

                    ColumnLayout {
                        spacing: 10

                        RowLayout {
                            spacing: 10

                            Text {
                                text: "Server URL"
                                color: textColor
                                Layout.preferredWidth: 100
                            }

                            TextField {
                                id: serverUrlField
                                text: settingsManager.serverUrl
                                Layout.fillWidth: true
                                onTextChanged: {
                                    settingsManager.serverUrl = text
                                }
                            }
                        }

                        RowLayout {
                            spacing: 10

                            Button {
                                text: "Connect"
                                Layout.fillWidth: true
                                onClicked: {
                                    sensorClient.serverUrl = serverUrlField.text
                                    sensorClient.connectToServer()
                                }
                            }

                            Button {
                                text: "Disconnect"
                                Layout.fillWidth: true
                                onClicked: {
                                    sensorClient.disconnectFromServer()
                                }
                            }

                            Text {
                                text: sensorClient.connected ? "🟢 Connected" : "🔴 Disconnected"
                                color: sensorClient.connected ? successColor : errorColor
                                font.pixelSize: 12
                            }
                        }
                    }
                }

                // Actions
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Reset to Defaults"
                        Layout.fillWidth: true
                        onClicked: {
                            settingsManager.resetToDefaults()
                            themeManager.resetToDefaults()
                            themeSwitch.checked = settingsManager.darkTheme
                            refreshSpin.value = settingsManager.refreshInterval
                        }
                    }

                    Button {
                        text: "Save Settings"
                        Layout.fillWidth: true
                        highlighted: true
                        onClicked: {
                            settingsManager.save()
                            root.visible = false
                        }
                    }
                }

                Text {
                    text: "Settings saved to: " + settingsManager.getSettingsFile()
                    font.pixelSize: 10
                    color: secondaryTextColor
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }
    }

    // Close on escape
    Keys.onPressed: {
        if (event.key === Qt.Key_Escape) {
            root.visible = false
        }
    }
}
