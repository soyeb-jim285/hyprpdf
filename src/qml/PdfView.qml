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

    signal annotationClicked(string id, int type, point scenePt)
    signal textSelected(int page, var rects, point scenePt)
    signal stickyCreated(string id, int page, point scenePt)

    property int activeAnnotTool: 0        // 0 = none, 1 = sticky, 2 = ink
    property color activeAnnotColor: "#f9e2af"
    property real activeInkWidth: 2.5

    // Per-page selection rects (word bboxes in PDF points), keyed by page index.
    // Cleared on new drag press. Drawn as persistent highlight overlay.
    property var _selectionByPage: ({})
    property int _selectionRev: 0
    function _clearSelection() {
        _selectionByPage = ({})
        _selectionRev++
    }

    // Increments on every resultsChanged — forces Repeater model to re-evaluate
    property int _searchRev: 0

    // Manual signal wiring in addition to Connections — the declarative Connections
    // inside per-tab PdfView instances was silently not firing.
    property var _wiredController: null
    function _wireController() {
        if (_wiredController === root.searchController) return
        _wiredController = root.searchController
        if (!_wiredController) return
        _wiredController.resultsChanged.connect(function () {
            root._searchRev++
        })
        _wiredController.currentChanged.connect(function () {
            root._searchRev++
        })
    }
    Component.onCompleted: _wireController()
    onSearchControllerChanged: _wireController()

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
                    id: invertedPage
                    anchors.fill: parent
                    active: root.invertColors
                    sourceComponent: ShaderEffect {
                        property variant src: img
                        fragmentShader: "qrc:/HyprPDF/qml/shaders/invert.frag.qsb"
                    }
                }

                Repeater {
                    id: matchRepeater
                    model: {
                        root._searchRev
                        if (!root.searchController) return []
                        // Force the binding through a local QML property because the
                        // shared SearchController QObject is passed around as `var`.
                        root.searchController.revision
                        return root.searchController.matchesOnPage(index)
                    }
                    delegate: Item {
                        id: matchRect
                        required property var modelData
                        x: modelData.x * pageItem.pxPerPt
                        y: modelData.y * pageItem.pxPerPt
                        width:  modelData.width  * pageItem.pxPerPt
                        height: modelData.height * pageItem.pxPerPt
                        visible: width > 0 && height > 0
                        opacity: isCurrent ? 0.60 : 0.55

                        readonly property bool isCurrent: {
                            root._searchRev
                            return (root.searchController
                                    && root.searchController.currentPage === index
                                    && root.searchController.currentRect.x === modelData.x
                                    && root.searchController.currentRect.y === modelData.y)
                        }

                        ShaderEffectSource {
                            id: matchSrc
                            anchors.fill: parent
                            live: true
                            hideSource: false
                            sourceItem: root.invertColors ? invertedPage : img
                            sourceRect: Qt.rect(matchRect.x, matchRect.y,
                                                matchRect.width, matchRect.height)
                        }
                        ShaderEffect {
                            anchors.fill: parent
                            property variant src: matchSrc
                            property color tintColor: matchRect.isCurrent ? "#f59e0b" : "#fde68a"
                            property vector2d boxSize: Qt.vector2d(matchRect.width, matchRect.height)
                            property real boxRadius: Math.min(4, matchRect.height / 3)
                            property real tintStrength: 0.85
                            fragmentShader: "qrc:/HyprPDF/qml/shaders/highlight_under_text.frag.qsb"
                        }
                    }
                }


                // Persistent selection highlight overlay (word-level bboxes).
                Repeater {
                    model: {
                        const rev = root._selectionRev
                        const m = root._selectionByPage
                        return (m && m[index]) ? m[index] : []
                    }
                    delegate: Item {
                        id: selRect
                        required property var modelData
                        x: modelData.x * pageItem.pxPerPt
                        y: modelData.y * pageItem.pxPerPt
                        width:  modelData.width  * pageItem.pxPerPt
                        height: modelData.height * pageItem.pxPerPt
                        visible: width > 0 && height > 0
                        opacity: 0.45

                        ShaderEffectSource {
                            id: selSrc
                            anchors.fill: parent
                            live: true
                            hideSource: false
                            sourceItem: root.invertColors ? invertedPage : img
                            sourceRect: Qt.rect(selRect.x, selRect.y,
                                                selRect.width, selRect.height)
                        }
                        ShaderEffect {
                            anchors.fill: parent
                            property variant src: selSrc
                            property color tintColor: Theme.accent
                            property vector2d boxSize: Qt.vector2d(selRect.width, selRect.height)
                            property real boxRadius: Math.min(3, selRect.height / 4)
                            property real tintStrength: 0.55
                            fragmentShader: "qrc:/HyprPDF/qml/shaders/highlight_under_text.frag.qsb"
                        }
                    }
                }

                MouseArea {
                    id: selArea
                    anchors.fill: parent
                    enabled: root.activeAnnotTool === 0 || root.activeAnnotTool === 3
                    property point startPt
                    property rect  rubberband
                    acceptedButtons: Qt.LeftButton
                    cursorShape: root.activeAnnotTool === 3 ? Qt.IBeamCursor : Qt.ArrowCursor
                    preventStealing: true
                    propagateComposedEvents: false

                    property point endPt
                    function _updateSelectionLive() {
                        if (!root.document) return
                        if (rubberband.width < 2 || rubberband.height < 2) return
                        const sxPt = startPt.x / pageItem.pxPerPt
                        const syPt = startPt.y / pageItem.pxPerPt
                        const exPt = endPt.x / pageItem.pxPerPt
                        const eyPt = endPt.y / pageItem.pxPerPt
                        const wordRects = root.document.selectionRectsLine(
                                index, sxPt, syPt, exPt, eyPt)
                        const m = root._selectionByPage
                        m[index] = wordRects
                        root._selectionByPage = m
                        root._selectionRev++
                    }

                    onPressed: (m) => {
                        startPt = Qt.point(m.x, m.y)
                        endPt = Qt.point(m.x, m.y)
                        rubberband = Qt.rect(m.x, m.y, 0, 0)
                        root._clearSelection()
                    }
                    onPositionChanged: (m) => {
                        if (!pressed) return
                        endPt = Qt.point(m.x, m.y)
                        rubberband = Qt.rect(Math.min(startPt.x, m.x),
                                             Math.min(startPt.y, m.y),
                                             Math.abs(m.x - startPt.x),
                                             Math.abs(m.y - startPt.y))
                        _updateSelectionLive()
                    }
                    onReleased: {
                        if (rubberband.width < 3 || rubberband.height < 3) {
                            rubberband = Qt.rect(0, 0, 0, 0); return
                        }
                        if (root.document) {
                            const sxPt = startPt.x / pageItem.pxPerPt
                            const syPt = startPt.y / pageItem.pxPerPt
                            const exPt = endPt.x / pageItem.pxPerPt
                            const eyPt = endPt.y / pageItem.pxPerPt
                            const wordRects = root.document.selectionRectsLine(
                                    index, sxPt, syPt, exPt, eyPt)
                            const m = root._selectionByPage
                            m[index] = wordRects
                            root._selectionByPage = m
                            root._selectionRev++
                            let minX = Math.min(sxPt, exPt), maxX = Math.max(sxPt, exPt)
                            let minY = Math.min(syPt, eyPt), maxY = Math.max(syPt, eyPt)
                            const bbox = Qt.rect(minX, minY, maxX - minX, maxY - minY)
                            const txt = root.document.textInRect(index, bbox)
                            if (txt.length > 0 && typeof clipboard !== "undefined") {
                                clipboard.setText(txt)
                            }
                            // Bubble the selection up for the popup
                            if (wordRects.length > 0) {
                                const releasePt = mapToItem(null, selArea.rubberband.x + selArea.rubberband.width,
                                                                    selArea.rubberband.y + selArea.rubberband.height)
                                root.textSelected(index, wordRects, Qt.point(releasePt.x, releasePt.y))
                            }
                        }
                        rubberband = Qt.rect(0, 0, 0, 0)
                    }
                }

                // Rubberband hidden — live word-level selection replaces it

                // Tool-aware MouseArea for sticky + ink (takes priority when active)
                MouseArea {
                    id: toolArea
                    anchors.fill: parent
                    enabled: root.activeAnnotTool === 1 || root.activeAnnotTool === 2 || root.activeAnnotTool === 5
                    cursorShape: Qt.CrossCursor
                    acceptedButtons: Qt.LeftButton
                    preventStealing: true

                    property var currentStroke: []

                    onPressed: (m) => {
                        if (root.activeAnnotTool === 1) {
                            const ptPts = Qt.point(m.x / pageItem.pxPerPt, m.y / pageItem.pxPerPt)
                            const scenePt = pageItem.mapToItem(null, m.x, m.y)
                            const id = annotationStore.addStickyNote(
                                    index, ptPts, root.activeAnnotColor, "")
                            root.stickyCreated(id, index, Qt.point(scenePt.x, scenePt.y))
                        } else if (root.activeAnnotTool === 5) {
                            const ptPts = Qt.point(m.x / pageItem.pxPerPt, m.y / pageItem.pxPerPt)
                            const scenePt = pageItem.mapToItem(null, m.x, m.y)
                            const id = annotationStore.addTextBox(
                                    index, ptPts, root.activeAnnotColor, "", 14.0)
                            root.stickyCreated(id, index, Qt.point(scenePt.x, scenePt.y))
                        } else if (root.activeAnnotTool === 2) {
                            currentStroke = [Qt.point(m.x / pageItem.pxPerPt,
                                                     m.y / pageItem.pxPerPt)]
                        }
                    }
                    onPositionChanged: (m) => {
                        if (!pressed || root.activeAnnotTool !== 2) return
                        const p = Qt.point(m.x / pageItem.pxPerPt, m.y / pageItem.pxPerPt)
                        const last = currentStroke[currentStroke.length - 1]
                        const dx = (p.x - last.x) * pageItem.pxPerPt
                        const dy = (p.y - last.y) * pageItem.pxPerPt
                        if ((dx * dx + dy * dy) >= 4) currentStroke.push(p)
                    }
                    onReleased: {
                        if (root.activeAnnotTool !== 2 || currentStroke.length < 2) {
                            currentStroke = []
                            return
                        }
                        const strokes = [currentStroke]
                        annotationStore.addInk(index, strokes,
                                                root.activeAnnotColor,
                                                root.activeInkWidth)
                        currentStroke = []
                    }
                }

                AnnotLayer {
                    anchors.fill: parent
                    z: 10
                    pageIndex: index
                    pxPerPt: pageItem.pxPerPt
                    pageImage: img
                    onAnnotationClicked: (id, type, scenePt) => {
                        annotationStore.selectedId = id
                        root.annotationClicked(id, type, scenePt)
                    }
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
