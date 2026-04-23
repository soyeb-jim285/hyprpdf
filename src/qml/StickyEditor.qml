import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HyprPDF 1.0

Rectangle {
    id: editor
    width: 320
    height: 220
    radius: Theme.radiusMedium
    color: Theme.mantle
    border.color: Theme.overlay
    border.width: 1
    visible: false
    z: 1000

    property string annotId: ""
    signal closed()

    function openFor(id, atX, atY, initialNote) {
        editor.annotId = id
        body.text = initialNote || ""
        editor.x = atX
        editor.y = atY
        editor.visible = true
        body.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        Text {
            text: "Sticky note"
            color: Theme.text
            font.pixelSize: Theme.fontNormal
            font.bold: true
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                id: body
                color: Theme.text
                placeholderText: "Type note body..."
                wrapMode: TextEdit.Wrap
                background: Rectangle { color: Theme.surface; radius: Theme.radiusSmall }
            }
        }
        RowLayout {
            spacing: 6
            Item { Layout.fillWidth: true }
            Button {
                text: "Delete"
                onClicked: {
                    annotationStore.remove(editor.annotId)
                    editor.visible = false
                    editor.closed()
                }
            }
            Button {
                text: "Save"
                onClicked: {
                    annotationStore.setNote(editor.annotId, body.text)
                    editor.visible = false
                    editor.closed()
                }
            }
        }
    }

    Keys.onEscapePressed: { editor.visible = false; editor.closed() }
}
