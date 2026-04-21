import QtQuick
import QtQuick.Controls
import HyprPDF 1.0

Item {
    id: root

    property var document: null
    property real zoom: 1.0
    property int  currentPage: 0
    property bool invertColors: false
    property var  searchController: null

    function zoomIn()   { zoom *= 1.2 }
    function zoomOut()  { zoom /= 1.2 }
    function fitWidth() {
        if (!document || document.pageCount <= 0) return
        const s = document.pageSize(0)
        if (s.width > 0) zoom = (root.width - 32) / (s.width * (96.0 / 72.0))
    }
    function fitPage() {
        if (!document || document.pageCount <= 0) return
        const s = document.pageSize(0)
        if (s.width > 0 && s.height > 0) {
            const pxW = s.width * (96.0 / 72.0)
            const pxH = s.height * (96.0 / 72.0)
            zoom = Math.min((root.width - 32) / pxW, (root.height - 32) / pxH)
        }
    }
    function scrollBy(dy) { list.contentY = Math.max(0, list.contentY + dy) }
    function nextPage()   { goToPage(currentPage + 1) }
    function prevPage()   { goToPage(currentPage - 1) }
    function goToPage(idx) {
        if (!document || document.pageCount <= 0) return
        const clamped = Math.max(0, Math.min(document.pageCount - 1, idx))
        list.positionViewAtIndex(clamped, ListView.Beginning)
        currentPage = clamped
    }
    function scrollToMatch(page, rect) { goToPage(page) }

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

        onContentYChanged: {
            const idx = list.indexAt(list.width / 2, contentY + height * 0.3)
            if (idx >= 0 && idx !== root.currentPage) root.currentPage = idx
        }

        delegate: Item {
            id: pageItem
            readonly property int targetW: {
                if (!root.document) return 0
                const s = root.document.pageSize(index)
                const pxW = s.width * (96.0 / 72.0)
                return Math.max(100, Math.round(pxW * root.zoom))
            }
            readonly property real pxPerPt: root.document && root.document.pageSize(index).width > 0
                                            ? (targetW / root.document.pageSize(index).width) : 1.0
            readonly property real pageAspectHeight: {
                if (!root.document) return 800
                const s = root.document.pageSize(index)
                return (s.width > 0) ? (targetW * (s.height / s.width)) : 800
            }

            width: list.width
            height: pageAspectHeight

            Rectangle {
                id: paper
                anchors.centerIn: parent
                width: pageItem.targetW
                height: pageItem.pageAspectHeight
                color: "white"
                border.color: Theme.surface
                border.width: 1

                Image {
                    id: img
                    anchors.fill: parent
                    source: root.document
                            ? "image://pdf/" + root.document.id + "/" + index + "/" + pageItem.targetW
                            : ""
                    asynchronous: true
                    cache: false
                    fillMode: Image.PreserveAspectFit
                    visible: !root.invertColors
                }

                Loader {
                    anchors.fill: parent
                    active: root.invertColors
                    sourceComponent: ShaderEffect {
                        property variant src: img
                        fragmentShader: "qrc:/HyprPDF/qml/shaders/invert.frag.qsb"
                    }
                }

                Repeater {
                    model: root.searchController
                           ? root.searchController.matchesOnPage(index)
                           : []
                    delegate: Rectangle {
                        required property var modelData
                        x: modelData.x * pageItem.pxPerPt
                        y: modelData.y * pageItem.pxPerPt
                        width:  modelData.width  * pageItem.pxPerPt
                        height: modelData.height * pageItem.pxPerPt
                        color: Theme.warning
                        opacity: 0.35
                        border.color: Theme.warning
                        border.width: (root.searchController
                                       && root.searchController.currentPage === index) ? 1 : 0
                    }
                }

                MouseArea {
                    id: selArea
                    anchors.fill: parent
                    property point startPt
                    property rect  rubberband
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.IBeamCursor
                    onPressed: (m) => {
                        startPt = Qt.point(m.x, m.y)
                        rubberband = Qt.rect(m.x, m.y, 0, 0)
                    }
                    onPositionChanged: (m) => {
                        if (!pressed) return
                        rubberband = Qt.rect(Math.min(startPt.x, m.x),
                                             Math.min(startPt.y, m.y),
                                             Math.abs(m.x - startPt.x),
                                             Math.abs(m.y - startPt.y))
                    }
                    onReleased: {
                        if (rubberband.width < 3 || rubberband.height < 3) {
                            rubberband = Qt.rect(0, 0, 0, 0); return
                        }
                        const rectPts = Qt.rect(rubberband.x / pageItem.pxPerPt,
                                                rubberband.y / pageItem.pxPerPt,
                                                rubberband.width  / pageItem.pxPerPt,
                                                rubberband.height / pageItem.pxPerPt)
                        const txt = root.document
                                    ? root.document.textInRect(index, rectPts) : ""
                        if (txt.length > 0 && typeof clipboard !== "undefined") {
                            clipboard.setText(txt)
                        }
                        rubberband = Qt.rect(0, 0, 0, 0)
                    }
                }

                Rectangle {
                    visible: selArea.rubberband.width > 0
                    x: selArea.rubberband.x; y: selArea.rubberband.y
                    width: selArea.rubberband.width; height: selArea.rubberband.height
                    color: Theme.accent; opacity: 0.25
                    border.color: Theme.accent; border.width: 1
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: img.status !== Image.Ready
                    visible: running && !root.invertColors
                }
            }
        }
    }

    Connections {
        target: root.searchController
        function onScrollToRequested(page, rect) { root.scrollToMatch(page, rect) }
    }
}
