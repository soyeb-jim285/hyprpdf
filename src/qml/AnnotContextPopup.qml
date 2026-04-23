import QtQuick
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: root
    property string annotId: ""
    property int annotType: 0
    signal editNoteRequested(string id, real sceneX, real sceneY)

    readonly property var swatches: ["#f9e2af", "#a6e3a1", "#f38ba8", "#89b4fa", "#fab387", "#cba6f7"]

    width: 280
    height: 40
    radius: Theme.radiusMedium
    color: Theme.surface
    border.color: Theme.overlay
    border.width: 1
    visible: false
    z: 500

    function showAt(id, type, sceneX, sceneY) {
        root.annotId = id
        root.annotType = type
        root.x = sceneX - width / 2
        root.y = sceneY + 8
        root.visible = true
    }
    function hide() { root.visible = false }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4
        Repeater {
            model: root.swatches
            delegate: Rectangle {
                required property string modelData
                implicitWidth: 22
                implicitHeight: 22
                radius: 11
                color: modelData
                border.color: Theme.overlay
                border.width: 1
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        annotationStore.setColor(root.annotId, modelData)
                        root.hide()
                    }
                }
            }
        }
        Rectangle { implicitWidth: 1; implicitHeight: 20; color: Theme.overlay }
        Rectangle {
            implicitWidth: 28
            implicitHeight: 28
            visible: root.annotType === 3
            radius: Theme.radiusSmall
            color: hh1.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10) : "transparent"
            HoverHandler { id: hh1 }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.editNoteRequested(root.annotId, root.x + root.width / 2, root.y)
                    root.hide()
                }
            }
            Text { anchors.centerIn: parent; text: "✎"; color: Theme.text; font.pixelSize: 16 }
        }
        Rectangle {
            implicitWidth: 28
            implicitHeight: 28
            radius: Theme.radiusSmall
            color: hh2.hovered ? Qt.rgba(Theme.error.r, Theme.error.g, Theme.error.b, 0.15) : "transparent"
            HoverHandler { id: hh2 }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    annotationStore.remove(root.annotId)
                    root.hide()
                }
            }
            Text { anchors.centerIn: parent; text: "🗑"; color: Theme.error; font.pixelSize: 14 }
        }
    }
}
