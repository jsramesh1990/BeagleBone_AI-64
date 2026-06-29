import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property string unit: ""
    property color color: primaryColor
    property real alertThreshold: 80
    property real barHeight: 20
    property real cornerRadius: 10

    property real normalizedValue: Math.max(0, Math.min(1, (value - minValue) / (maxValue - minValue)))
    property bool isAlert: value > alertThreshold

    height: barHeight + 30

    // Value text
    Text {
        id: valueText
        text: root.value.toFixed(1) + " " + root.unit
        font.pixelSize: 12
        color: root.isAlert ? errorColor : textColor
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 60
        horizontalAlignment: Text.AlignRight
    }

    // Background bar
    Rectangle {
        anchors.left: valueText.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: root.barHeight
        radius: root.cornerRadius
        color: borderColor
        clip: true

        // Value bar
        Rectangle {
            width: Math.max(0, parent.width * root.normalizedValue)
            height: parent.height
            radius: root.cornerRadius
            color: root.isAlert ? errorColor : root.color
            opacity: 0.8

            // Gradient
            gradient: Gradient {
                GradientStop { position: 0.0; color: root.isAlert ? errorColor : root.color }
                GradientStop { position: 1.0; color: root.isAlert ? errorColor : root.color }
            }
        }

        // Alert threshold marker
        Rectangle {
            x: parent.width * Math.max(0, Math.min(1, (root.alertThreshold - root.minValue) / (root.maxValue - root.minValue))) - 1
            width: 2
            height: parent.height
            color: errorColor
            visible: root.alertThreshold > root.minValue && root.alertThreshold < root.maxValue
        }
    }

    // Min/max labels
    Text {
        text: root.minValue.toFixed(0)
        font.pixelSize: 9
        color: secondaryTextColor
        anchors.left: valueText.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 2
    }

    Text {
        text: root.maxValue.toFixed(0)
        font.pixelSize: 9
        color: secondaryTextColor
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 2
    }
}
