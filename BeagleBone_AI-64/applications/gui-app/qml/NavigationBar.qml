import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: surfaceColor
    border.right.color: borderColor
    border.right.width: 1

    property string currentView: "dashboard"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 5

        // Logo/Title
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "transparent"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 5

                Text {
                    text: "📊"
                    font.pixelSize: 32
                    Layout.alignment: Qt.AlignCenter
                }

                Text {
                    text: "Dashboard"
                    font.pixelSize: 14
                    font.bold: true
                    color: primaryColor
                    Layout.alignment: Qt.AlignCenter
                }

                Text {
                    text: "v1.0.0"
                    font.pixelSize: 10
                    color: secondaryTextColor
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }

        // Navigation items
        NavigationItem {
            id: dashboardItem
            icon: "🏠"
            label: "Dashboard"
            selected: root.currentView === "dashboard"
            onClicked: {
                root.currentView = "dashboard"
                dashboard.showView("dashboard")
            }
        }

        NavigationItem {
            id: sensorsItem
            icon: "📡"
            label: "Sensors"
            selected: root.currentView === "sensors"
            onClicked: {
                root.currentView = "sensors"
                dashboard.showView("sensors")
            }
        }

        NavigationItem {
            id: chartsItem
            icon: "📈"
            label: "Charts"
            selected: root.currentView === "charts"
            onClicked: {
                root.currentView = "charts"
                dashboard.showView("charts")
            }
        }

        NavigationItem {
            id: devicesItem
            icon: "🔌"
            label: "Devices"
            selected: root.currentView === "devices"
            onClicked: {
                root.currentView = "devices"
                dashboard.showView("devices")
            }
        }

        NavigationItem {
            id: settingsItem
            icon: "⚙️"
            label: "Settings"
            selected: root.currentView === "settings"
            onClicked: {
                root.currentView = "settings"
                settingsPanel.visible = true
            }
        }

        // Spacer
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // Footer
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }

        Text {
            text: "BBB AI64"
            font.pixelSize: 10
            color: secondaryTextColor
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 10
        }
    }

    // Navigation Item Component
    component NavigationItem : Rectangle {
        id: navItem
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        color: selected ? Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1) : "transparent"
        radius: 5

        property string icon: ""
        property string label: ""
        property bool selected: false

        signal clicked()

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            spacing: 10

            Text {
                text: navItem.icon
                font.pixelSize: 20
                color: navItem.selected ? primaryColor : secondaryTextColor
            }

            Text {
                text: navItem.label
                font.pixelSize: 13
                font.bold: navItem.selected
                color: navItem.selected ? primaryColor : textColor
                Layout.fillWidth: true
            }

            Rectangle {
                width: 3
                height: 20
                radius: 1.5
                color: primaryColor
                visible: navItem.selected
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: navItem.clicked()
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
        }

        Rectangle {
            anchors.fill: parent
            color: primaryColor
            opacity: 0.05
            visible: parent.containsMouse && !navItem.selected
            radius: 5
        }
    }
}
