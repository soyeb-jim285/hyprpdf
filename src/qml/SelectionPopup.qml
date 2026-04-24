import QtQuick
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: root
    property int page: 0
    property var rects: []
    // Color comes from AnnotToolbar's active color (no swatches in popup)
    property color activeColor: "#f9e2af"

    width: 132
    height: 38
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
        signal clicked()
        implicitWidth: 36
        implicitHeight: 30
        radius: Theme.radiusSmall
        color: hh.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10) : "transparent"
        HoverHandler { id: hh }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4
        TypeBtn {
            IconHighlighter { size: 18; color: Theme.text; anchors.centerIn: parent }
            onClicked: {
                annotationStore.addHighlight(root.page, root.rects, root.activeColor)
                root.created(); root.hide()
            }
        }
        TypeBtn {
            IconUnderline { size: 18; color: Theme.text; anchors.centerIn: parent }
            onClicked: {
                annotationStore.addUnderline(root.page, root.rects, root.activeColor)
                root.created(); root.hide()
            }
        }
        TypeBtn {
            IconStrikethrough { size: 18; color: Theme.text; anchors.centerIn: parent }
            onClicked: {
                annotationStore.addStrikeout(root.page, root.rects, root.activeColor)
                root.created(); root.hide()
            }
        }
    }
}
