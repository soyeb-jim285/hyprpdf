import QtQuick
import QtQuick.Controls
import Quill 1.0 as Q
import HyprPDF 1.0

Rectangle {
    id: root
    property var outlineModel: null
    signal pageActivated(int page)

    color: Theme.mantle
    implicitWidth: 240

    Text {
        anchors.centerIn: parent
        visible: !root.outlineModel || tree.rows === 0
        text: "No outline"
        color: Theme.subtext
        opacity: 0.6
    }

    TreeView {
        id: tree
        anchors.fill: parent
        anchors.margins: 8
        clip: true
        model: root.outlineModel

        delegate: TreeViewDelegate {
            indentation: 14
            implicitHeight: 28
            contentItem: Text {
                text: (model && model.title) ? model.title : ""
                color: Theme.text
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: hoverHandler.hovered ? Theme.surface : "transparent"
                radius: Theme.radiusSmall
                HoverHandler { id: hoverHandler }
            }
            TapHandler {
                onTapped: {
                    const p = (model && model.page !== undefined) ? model.page : -1
                    if (p >= 0) root.pageActivated(p)
                }
            }
        }
    }
}
