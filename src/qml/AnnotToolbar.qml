import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: root
    implicitHeight: expanded ? 40 : 0
    property bool expanded: true

    // 0 = Hand (default, text selection + pan)
    // 1 = Sticky
    // 2 = Pen (ink)
    // 3 = Highlight (drag-select applies current style immediately)
    property int activeTool: 0

    // 0 = Highlight, 1 = Underline, 2 = Strikethrough (applied when activeTool === 3)
    property int highlightStyle: 0

    property color activeColor: "#f9e2af"
    property real activeInkWidth: 2.5

    color: Theme.crust
    clip: true
    Behavior on implicitHeight { NumberAnimation { duration: Theme.animDurationFast } }

    readonly property var swatches: ["#f9e2af", "#a6e3a1", "#f38ba8", "#89b4fa", "#fab387", "#cba6f7"]

    function resetToHand() { root.activeTool = 0 }

    component IBtn: Rectangle {
        property bool active: false
        signal clicked()
        implicitWidth: 30; implicitHeight: 30
        radius: Theme.radiusMedium
        color: active ? Theme.surface
             : hh.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.08)
             : "transparent"
        HoverHandler { id: hh }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }

    component Sep: Rectangle {
        implicitWidth: 1
        implicitHeight: 22
        Layout.alignment: Qt.AlignVCenter
        color: Theme.overlay
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 4

        IBtn {
            active: root.activeTool === 0
            onClicked: root.activeTool = 0
            IconHand { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 3
            onClicked: root.activeTool = 3
            IconHighlighter { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 2
            onClicked: root.activeTool = 2
            IconPen { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 1
            onClicked: root.activeTool = 1
            IconPin { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            active: root.activeTool === 5
            onClicked: root.activeTool = 5
            IconType { size: 18; color: Theme.text; anchors.centerIn: parent }
        }

        Sep {}

        RowLayout {
            spacing: 2
            visible: root.activeTool === 3
            IBtn {
                active: root.highlightStyle === 0
                onClicked: root.highlightStyle = 0
                IconHighlighter { size: 16; color: Theme.text; anchors.centerIn: parent }
            }
            IBtn {
                active: root.highlightStyle === 1
                onClicked: root.highlightStyle = 1
                IconUnderline { size: 16; color: Theme.text; anchors.centerIn: parent }
            }
            IBtn {
                active: root.highlightStyle === 2
                onClicked: root.highlightStyle = 2
                IconStrikethrough { size: 16; color: Theme.text; anchors.centerIn: parent }
            }
        }

        Sep { visible: root.activeTool === 3 }

        RowLayout {
            spacing: 4
            visible: root.activeTool !== 0
            Repeater {
                model: root.swatches
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
        }

        Sep { visible: root.activeTool === 2 }

        RowLayout {
            spacing: 6
            visible: root.activeTool === 2
            Text {
                text: "Width"
                color: Theme.subtext
                font.pixelSize: Theme.fontSmall
            }
            Slider {
                Layout.preferredWidth: 80
                from: 1; to: 6
                value: root.activeInkWidth
                onMoved: root.activeInkWidth = value
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: {
                switch (root.activeTool) {
                    case 0: return "Hand"
                    case 1: return "Sticky"
                    case 2: return "Pen"
                    case 3: return "Highlight"
                }
                return ""
            }
            color: Theme.subtext
            font.pixelSize: Theme.fontSmall
        }
    }
}
