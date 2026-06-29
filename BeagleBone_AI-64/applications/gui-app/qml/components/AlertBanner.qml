import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: parent.width * 0.8
    height: 60
    radius: 10
    color: typeColor
    opacity: 0.95
    visible: false

    property string message: ""
    property string type: "info" // info, warning, error, success
    property int displayTime: 5000

    property color typeColor: {
        switch(type) {
            case "info": return primaryColor
            case "warning": return warningColor
            case "error": return errorColor
            case "success": return successColor
            default: return primaryColor
        }
    }
    property string icon: {
        switch(type) {
            case "info": return "ℹ️"
            case "warning": return "⚠️"
            case "error": return "❌"
            case "success": return "✅"
            default: return "ℹ️"
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text {
            text: root.icon
            font.pixelSize: 24
        }

        Text {
            text: root.message
            font.pixelSize: 14
            color: "white"
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        Button {
            text: "✕"
            flat: true
            onClicked: {
                root.visible = false
                hideTimer.stop()
            }
        }
    }

    Timer {
        id: hideTimer
        interval: root.displayTime
        onTriggered: {
            root.visible = false
        }
    }

    function showMessage(msg, type) {
        root.message = msg
        root.type = type || "info"
        root.visible = true
        hideTimer.restart()
    }

    // Drop shadow
    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: 2
        radius: 8
        samples: 16
        color: shadowColor
    }
}
