import QtQuick
import QtQuick.Controls
import HyprPDF 1.0

Item {
    id: root

    // PdfDoc QObject. Exposes: id, pageCount, title, pageSize(i), text(i).
    property var document: null

    property real zoom: 1.0
    property int  currentPage: 0

    function zoomIn()   { zoom = zoom * 1.2 }
    function zoomOut()  { zoom = zoom / 1.2 }
    function fitWidth() {
        if (!root.document || root.document.pageCount <= 0) return
        var s = root.document.pageSize(0)
        if (s.width > 0)
            zoom = (root.width - 32) / (s.width * (96.0 / 72.0))
    }
    function fitPage() {
        if (!root.document || root.document.pageCount <= 0) return
        var s = root.document.pageSize(0)
        if (s.width > 0 && s.height > 0) {
            var pxW = s.width  * (96.0 / 72.0)
            var pxH = s.height * (96.0 / 72.0)
            zoom = Math.min((root.width - 32) / pxW, (root.height - 32) / pxH)
        }
    }
    function scrollBy(dy) {
        list.contentY = Math.max(0, list.contentY + dy)
    }
    function nextPage() {
        if (root.document)
            list.positionViewAtIndex(
                Math.min(root.document.pageCount - 1, root.currentPage + 1),
                ListView.Beginning)
    }
    function prevPage() {
        if (root.document)
            list.positionViewAtIndex(
                Math.max(0, root.currentPage - 1),
                ListView.Beginning)
    }

    // Empty-state label
    Text {
        anchors.centerIn: parent
        visible: !root.document
        text: "No document"
        color: Theme.subtext
        opacity: 0.6
    }

    ListView {
        id: list
        anchors.fill: parent
        visible: !!root.document
        model: root.document ? root.document.pageCount : 0
        spacing: 8
        clip: true
        cacheBuffer: 2000
        onCurrentIndexChanged: root.currentPage = currentIndex

        delegate: Item {
            id: pageItem

            readonly property int targetW: {
                if (!root.document) return 0
                var s = root.document.pageSize(index)
                var pxW = s.width * (96.0 / 72.0)
                return Math.max(100, Math.round(pxW * root.zoom))
            }
            readonly property real pageAspect: {
                if (!root.document) return 1.0
                var s = root.document.pageSize(index)
                return (s.height > 0 && s.width > 0)
                    ? (s.height / s.width)
                    : (792.0 / 612.0)
            }

            width:  list.width
            height: targetW > 0 ? Math.round(targetW * pageAspect) : 800

            Rectangle {
                anchors.centerIn: parent
                width:  pageItem.targetW
                height: pageItem.height
                color: "white"
                border.color: Theme.surface
                border.width: 1

                Image {
                    id: img
                    anchors.fill: parent
                    source: root.document
                            ? ("image://pdf/" + root.document.id
                               + "/" + index
                               + "/" + pageItem.targetW)
                            : ""
                    asynchronous: true
                    cache: false
                    fillMode: Image.PreserveAspectFit
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: img.status !== Image.Ready
                    visible: running
                }
            }
        }
    }
}
