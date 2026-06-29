import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: dashboard.title

    property bool isDarkTheme: themeManager.isDarkTheme

    // Theme colors
    property color backgroundColor: isDarkTheme ? "#1a1a2e" : "#f5f5f5"
    property color surfaceColor: isDarkTheme ? "#16213e" : "#ffffff"
    property color primaryColor: "#00b4d8"
    property color textColor: isDarkTheme ? "#ffffff" : "#333333"
    property color secondaryTextColor: isDarkTheme ? "#aaaaaa" : "#666666"
    property color accentColor: "#ff6b6b"
    property color successColor: "#4ecdc4"
    property color warningColor: "#ffd93d"

    // Dashboard instance
    property var dashboard: Dashboard {
        id: dashboard
    }

    // Theme management
    Component.onCompleted: {
        themeManager.initialize()
        setupTheme()
    }

    function setupTheme() {
        // Apply theme colors
        // ThemeManager handles the actual QML theme
    }

    // Main layout
    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        // Navigation bar
        NavigationBar {
            id: navigationBar
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            onViewSelected: {
                dashboard.showView(view)
                stackView.push(view, {})
            }
        }

        // Main content area
        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true

            initialItem: Dashboard {}

            Connections {
                target: dashboard
                function onViewChanged(view) {
                    // Handle view changes
                }
            }
        }

        // Status bar
        StatusBar {
            id: statusBar
            Layout.preferredHeight: 30
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }
    }

    // Alert banner (shown when needed)
    AlertBanner {
        id: alertBanner
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        z: 10
    }

    // Settings panel (popup)
    SettingsPanel {
        id: settingsPanel
        anchors.centerIn: parent
        visible: false
        z: 20
        width: 400
        height: 500
    }

    // Overlay for popups
    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "black"
        opacity: 0.5
        visible: settingsPanel.visible
        z: 19
        MouseArea {
            anchors.fill: parent
            onClicked: settingsPanel.visible = false
        }
    }

    // Keyboard shortcuts
    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: dashboard.exitApplication()
    }

    Shortcut {
        sequence: "F11"
        onActivated: dashboard.toggleFullScreen()
    }

    Shortcut {
        sequence: "Ctrl+T"
        onActivated: dashboard.toggleTheme()
    }

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: settingsPanel.visible = !settingsPanel.visible
    }

    Shortcut {
        sequence: "Ctrl+1"
        onActivated: dashboard.showView("dashboard")
    }

    Shortcut {
        sequence: "Ctrl+2"
        onActivated: dashboard.showView("sensors")
    }

    Shortcut {
        sequence: "Ctrl+3"
        onActivated: dashboard.showView("charts")
    }

    Shortcut {
        sequence: "Ctrl+4"
        onActivated: dashboard.showView("devices")
    }
}
