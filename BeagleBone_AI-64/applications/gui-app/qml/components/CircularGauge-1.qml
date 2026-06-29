import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Item {
    id: root
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property string unit: ""
    property real alertThreshold: 80
    property color color: primaryColor
    property real lineWidth: 8
    property real startAngle: -135
    property real endAngle: 135

    property real normalizedValue: Math.max(0, Math.min(1, (value - minValue) / (maxValue - minValue)))
    property real valueAngle: startAngle + (endAngle - startAngle) * normalizedValue
    property bool isAlert: value > alertThreshold

    // Background arc
    Shape {
        anchors.fill: parent
        layer.enabled: true

        ShapePath {
            strokeWidth: root.lineWidth
            strokeColor: borderColor
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: parent.width / 2
                centerY: parent.height / 2
                radiusX: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                radiusY: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                startAngle: root.startAngle
                sweepAngle: root.endAngle - root.startAngle
            }
        }

        // Value arc
        ShapePath {
            strokeWidth: root.lineWidth
            strokeColor: root.isAlert ? errorColor : root.color
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: parent.width / 2
                centerY: parent.height / 2
                radiusX: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                radiusY: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                startAngle: root.startAngle
                sweepAngle: root.valueAngle - root.startAngle
            }
        }

        // Alert threshold arc
        ShapePath {
            strokeWidth: root.lineWidth
            strokeColor: errorColor
            fillColor: "transparent"
            strokeStyle: ShapePath.DashLine
            dashPattern: [4, 4]
            visible: root.alertThreshold > root.minValue && root.alertThreshold < root.maxValue

            PathAngleArc {
                centerX: parent.width / 2
                centerY: parent.height / 2
                radiusX: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                radiusY: Math.min(parent.width, parent.height) / 2 - root.lineWidth
                startAngle: root.startAngle + (root.endAngle - root.startAngle) * 
                           ((root.alertThreshold - root.minValue) / (root.maxValue - root.minValue))
                sweepAngle: 2
            }
        }
    }

    // Center value text
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 2

        Text {
            text: root.value.toFixed(1)
            font.pixelSize: Math.min(parent.parent.width, parent.parent.height) * 0.2
            font.bold: true
            color: root.isAlert ? errorColor : textColor
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

    // Min/max labels
    Text {
        text: root.minValue.toFixed(0)
        font.pixelSize: 10
        color: secondaryTextColor
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
    }

    Text {
        text: root.maxValue.toFixed(0)
        font.pixelSize: 10
        color: secondaryTextColor
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
    }

    // Alert threshold label
    Text {
        text: "⚠ " + root.alertThreshold.toFixed(0)
        font.pixelSize: 9
        color: errorColor
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 10
        visible: root.alertThreshold > root.minValue && root.alertThreshold < root.maxValue
    }
}
