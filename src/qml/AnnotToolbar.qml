import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: root
    implicitHeight: expanded ? 36 : 0
    property bool expanded: false
    property int activeTool: 0       // 0 = none, 1 = sticky, 2 = ink
    property color activeColor: "#f9e2af"
    property real activeInkWidth: 2.5

    color: Theme.crust
    clip: true
    Behavior on implicitHeight { NumberAnimation { duration: Theme.animDurationFast } }

    readonly property var palette: ["#f9e2af", "#a6e3a1", "#f38ba8", "#89b4fa", "#fab387", "#cba6f7"]

    component IBtn: Rectangle {
        property bool active: false
        signal clicked()
        implicitWidth: 28; implicitHeight: 28
        radius: Theme.radiusMedium
        color: active ? Theme.surface
             : hh.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.08)
             : "transparent"
        HoverHandler { id: hh }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 4

        IBtn {
            active: root.activeTool === 1
            onClicked: root.activeTool = root.activeTool === 1 ? 0 : 1
            IconPin { size: 16; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 2
            onClicked: root.activeTool = root.activeTool === 2 ? 0 : 2
            IconPen { size: 16; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 0
            onClicked: root.activeTool = 0
            IconX { size: 16; color: Theme.text; anchors.centerIn: parent }
        }

        Rectangle { implicitWidth: 1; implicitHeight: 20; color: Theme.overlay }

        Repeater {
            model: root.palette
            delegate: Rectangle {
                required property string modelData
                implicitWidth: 22; implicitHeight: 22
                radius: 11
                color: modelData
                border.color: root.activeColor === modelData ? Theme.text : Theme.overlay
                border.width: root.activeColor === modelData ? 2 : 1
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.activeColor = modelData
                }
            }
        }

        Rectangle {
            visible: root.activeTool === 2
            implicitWidth: 1
            implicitHeight: 20
            color: Theme.overlay
        }

        Text {
            visible: root.activeTool === 2
            text: "Width:"
            color: Theme.subtext
            font.pixelSize: Theme.fontSmall
        }
        Slider {
            id: widthSlider
            visible: root.activeTool === 2
            Layout.preferredWidth: 80
            from: 1; to: 6; value: root.activeInkWidth
            onMoved: root.activeInkWidth = value
        }

        Item { Layout.fillWidth: true }
    }
}
