import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "transparent"
    border.color: borderColor
    border.width: 1
    radius: 5

    property string label: ""
    property string value: ""
    property string unit: ""
    property string icon: ""
    property color valueColor: textColor
    property bool bold: true
    property bool showChange: false
    property real changeValue: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 2

        RowLayout {
            spacing: 5

            Text {
                text: root.icon
                font.pixelSize: 16
                visible: root.icon !== ""
            }

            Text {
                text: root.label
                font.pixelSize: 11
                color: secondaryTextColor
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }

        RowLayout {
            spacing: 5

            Text {
                text: root.value
                font.pixelSize: 20
                font.bold: root.bold
                color: root.valueColor
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Text {
                text: root.unit
                font.pixelSize: 14
                color: secondaryTextColor
                visible: root.unit !== ""
            }

            Text {
                text: root.showChange ? (root.changeValue > 0 ? "↑" : "↓") : ""
                font.pixelSize: 16
                color: root.changeValue > 0 ? successColor : errorColor
                visible: root.showChange && root.changeValue !== 0
            }
        }

        // Change indicator
        Text {
            text: root.showChange ? (root.changeValue > 0 ? "+" : "") + root.changeValue.toFixed(1) + "%" : ""
            font.pixelSize: 10
            color: root.changeValue > 0 ? successColor : errorColor
            visible: root.showChange && root.changeValue !== 0
            Layout.alignment: Qt.AlignRight
        }
    }
}
