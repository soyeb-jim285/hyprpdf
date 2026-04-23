import QtQuick
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: root
    property int page: 0
    property var rects: []
    readonly property var swatches: ["#f9e2af", "#a6e3a1", "#f38ba8", "#89b4fa", "#fab387", "#cba6f7"]
    property color activeColor: swatches[0]

    width: 280
    height: 72
    radius: Theme.radiusMedium
    color: Theme.surface
    border.color: Theme.overlay
    border.width: 1
    visible: false
    z: 200

    signal created()

    function showAt(sceneX, sceneY, pg, rs) {
        root.page = pg
        root.rects = rs
        root.x = sceneX - width / 2
        root.y = sceneY + 8
        root.visible = true
    }
    function hide() { root.visible = false }

    component TypeBtn: Rectangle {
        property string label: ""
        signal clicked()
        implicitWidth: 76
        implicitHeight: 28
        radius: Theme.radiusSmall
        color: hh.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10) : "transparent"
        HoverHandler { id: hh }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
        Text {
            anchors.centerIn: parent
            text: parent.label
            color: Theme.text
            font.pixelSize: Theme.fontSmall
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            TypeBtn {
                label: "Highlight"
                onClicked: {
                    annotationStore.addHighlight(root.page, root.rects, root.activeColor)
                    root.created(); root.hide()
                }
            }
            TypeBtn {
                label: "Underline"
                onClicked: {
                    annotationStore.addUnderline(root.page, root.rects, root.activeColor)
                    root.created(); root.hide()
                }
            }
            TypeBtn {
                label: "Strikeout"
                onClicked: {
                    annotationStore.addStrikeout(root.page, root.rects, root.activeColor)
                    root.created(); root.hide()
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 6
            Repeater {
                model: root.swatches
                delegate: Rectangle {
                    required property string modelData
                    implicitWidth: 22
                    implicitHeight: 22
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
    }
}
