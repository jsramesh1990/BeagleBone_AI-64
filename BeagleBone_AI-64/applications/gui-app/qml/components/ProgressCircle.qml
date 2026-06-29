import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Item {
    id: root
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property string label: ""
    property string unit: "%"
    property color color: primaryColor
    property real lineWidth: 8
    property bool animated: true

    property real normalizedValue: Math.max(0, Math.min(1, (value - minValue) / (maxValue - minValue)))
    property real angle: 360 * normalizedValue

    width: 100
    height: 100

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
                radiusX: parent.width / 2 - root.lineWidth
                radiusY: parent.height / 2 - root.lineWidth
                startAngle: -90
                sweepAngle: 360
            }
        }

        ShapePath {
            id: progressPath
            strokeWidth: root.lineWidth
            strokeColor: root.color
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: parent.width / 2
                centerY: parent.height / 2
                radiusX: parent.width / 2 - root.lineWidth
                radiusY: parent.height / 2 - root.lineWidth
                startAngle: -90
                sweepAngle: root.animated ? progressAnimation.value : root.angle
            }

            NumberAnimation {
                id: progressAnimation
                target: progressPath
                property: "sweepAngle"
                from: 0
                to: root.angle
                duration: 500
                easing.type: Easing.InOutCubic
                running: root.animated && root.visible
                onRunningChanged: {
                    if (!running) {
                        progressPath.sweepAngle = root.angle
                    }
                }
            }
        }
    }

    // Center text
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 2

        Text {
            text: (root.value / root.maxValue * 100).toFixed(0) + "%"
            font.pixelSize: 18
            font.bold: true
            color: root.value > 80 ? errorColor : textColor
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignCenter
        }

        Text {
            text: root.label
            font.pixelSize: 10
            color: secondaryTextColor
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignCenter
        }
    }

    // Animation trigger
    onValueChanged: {
        if (animated) {
            progressAnimation.start()
        }
    }

    Component.onCompleted: {
        if (animated) {
            progressAnimation.start()
        }
    }
}
