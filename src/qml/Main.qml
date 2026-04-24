import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quill 1.0 as Q
import HyprPDF 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 820
    visible: true
    color: Theme.base
    title: "HyprPDF"

    property string initialFile: ""
    property bool leftPanelVisible: true
    property bool rightPanelVisible: true
    property bool invertColors: false
    property bool annotToolbarExpanded: true
    readonly property var sharedSearchController: searchController
    readonly property var sharedOutlineModel: outlineModel

    Component.onCompleted: {
        Q.Theme.background       = Qt.binding(() => Theme.base)
        Q.Theme.backgroundAlt    = Qt.binding(() => Theme.mantle)
        Q.Theme.backgroundDeep   = Qt.binding(() => Theme.crust)
        Q.Theme.surface0         = Qt.binding(() => Theme.surface)
        Q.Theme.surface1         = Qt.binding(() => Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10))
        Q.Theme.surface2         = Qt.binding(() => Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.15))
        Q.Theme.textPrimary      = Qt.binding(() => Theme.text)
        Q.Theme.textSecondary    = Qt.binding(() => Theme.subtext)
        Q.Theme.textTertiary     = Qt.binding(() => Theme.muted)
        Q.Theme.primary          = Qt.binding(() => Theme.accent)
        Q.Theme.success          = Qt.binding(() => Theme.success)
        Q.Theme.warning          = Qt.binding(() => Theme.warning)
        Q.Theme.error            = Qt.binding(() => Theme.error)
        Q.Theme.radiusSm         = Qt.binding(() => Theme.radiusSmall)
        Q.Theme.radius           = Qt.binding(() => Theme.radiusMedium)
        Q.Theme.radiusLg         = Qt.binding(() => Theme.radiusLarge)
        Q.Theme.fontSize         = Qt.binding(() => Theme.fontNormal)
        Q.Theme.fontSizeSmall    = Qt.binding(() => Theme.fontSmall)
        Q.Theme.fontSizeLarge    = Qt.binding(() => Theme.fontLarge)
        Q.Theme.animDuration     = Qt.binding(() => Theme.animDuration)
        Q.Theme.animDurationFast = Qt.binding(() => Theme.animDurationFast)
        Q.Theme.animDurationSlow = Qt.binding(() => Theme.animDurationSlow)

        if (initialFile.length > 0) documentController.open(initialFile)
    }

    Connections {
        target: documentController
        function onOpenFailed(reason) { console.warn("HyprPDF open failed:", reason) }
    }

    Connections {
        target: documentModel
        function onCurrentIndexChanged() {
            const cur = documentModel.currentIndex
            if (cur < 0) {
                searchController.setDocument(null)
                outlineModel.setDocument(null)
                return
            }
            const doc = documentModel.index(cur, 0).data(261)
            searchController.setDocument(doc)
            outlineModel.setDocument(doc)
        }
    }

    FileDialog {
        id: openDialog
        title: "Open document"
        nameFilters: [
            "Documents (*.pdf *.pptx *.ppt *.odp *.docx *.doc *.odt *.rtf *.epub *.md *.html)",
            "PDF (*.pdf)",
            "All files (*)"
        ]
        onAccepted: documentController.openFromUrl(selectedFile)
    }

    function curView() {
        if (documentModel.count === 0) return null
        const i = stack.currentIndex
        if (i <= 0) return null
        return stack.itemAt(i)
    }

    Action { id: actOpen;      text: "Open...";    shortcut: "Ctrl+O"; onTriggered: openDialog.open() }
    Action { id: actNewTab;    text: "New tab";    shortcut: "Ctrl+T"; onTriggered: openDialog.open() }
    Action { id: actCloseTab;  text: "Close tab";  shortcut: "Ctrl+W"; onTriggered: documentController.closeCurrent() }
    Action { id: actQuit;      text: "Quit";       shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
    Action { id: actZoomIn;    text: "Zoom in";    shortcut: "Ctrl++"; onTriggered: { var v = curView(); if (v) v.zoomIn() } }
    Action { id: actZoomOut;   text: "Zoom out";   shortcut: "Ctrl+-"; onTriggered: { var v = curView(); if (v) v.zoomOut() } }
    Action { id: actFitWidth;  text: "Fit width";  shortcut: "Ctrl+1"; onTriggered: { var v = curView(); if (v) v.fitWidth() } }
    Action { id: actFitPage;   text: "Fit page";   shortcut: "Ctrl+2"; onTriggered: { var v = curView(); if (v) v.fitPage() } }
    Action { id: actFind;      text: "Find";       shortcut: "Ctrl+F"; onTriggered: searchBar.openWithFocus() }
    Action { id: actFindNext;  text: "Next match"; shortcut: "F3";     onTriggered: searchController.next() }
    Action { id: actFindPrev;  text: "Prev match"; shortcut: "Shift+F3"; onTriggered: searchController.prev() }
    Action { id: actInvert;    text: "Invert colors"; shortcut: "Ctrl+I"; onTriggered: root.invertColors = !root.invertColors }
    Action { id: actLeft;      text: "Thumbnails panel"; shortcut: "F9"; onTriggered: root.leftPanelVisible = !root.leftPanelVisible }
    Action { id: actRight;     text: "Outline panel";    shortcut: "F10"; onTriggered: root.rightPanelVisible = !root.rightPanelVisible }
    Action { id: actAnnotBar;  text: "Annotation toolbar"; shortcut: "F8"; onTriggered: root.annotToolbarExpanded = !root.annotToolbarExpanded }
    Action { id: actDelAnnot; shortcut: "Delete"
             onTriggered: if (annotationStore.selectedId)
                             annotationStore.remove(annotationStore.selectedId) }
    Action { id: actUndo;     shortcut: "Ctrl+Z"; onTriggered: annotationStore.undo() }
    Action { id: actRedo1;    shortcut: "Ctrl+Shift+Z"; onTriggered: annotationStore.redo() }
    Action { id: actRedo2;    shortcut: "Ctrl+Y"; onTriggered: annotationStore.redo() }
    Action { id: actExport;   shortcut: "Ctrl+Shift+E"; onTriggered: exportAnnotDialog.open() }

    Shortcut { sequence: "/";       onActivated: searchBar.openWithFocus() }
    Shortcut { sequence: "Escape";  onActivated: {
        if (stickyEditor.visible) stickyEditor.visible = false
        if (annotCtxPopup.visible) annotCtxPopup.hide()
        if (selPopup.visible) selPopup.visible = false
        if (searchBar.visible) { searchBar.visible = false; searchController.clear() }
        // Always return to Hand tool on Escape
        annotToolbar.activeTool = 0
    } }
    Shortcut { sequence: "g,g";     onActivated: { var v = curView(); if (v) v.goToPage(0) } }
    Shortcut { sequence: "Shift+G"; onActivated: { var v = curView(); if (v && v.document) v.goToPage(v.document.pageCount - 1) } }
    Shortcut { sequence: "j";       onActivated: { var v = curView(); if (v) v.scrollBy(80) } }
    Shortcut { sequence: "k";       onActivated: { var v = curView(); if (v) v.scrollBy(-80) } }
    Shortcut { sequence: "PgDown";  onActivated: { var v = curView(); if (v) v.nextPage() } }
    Shortcut { sequence: "PgUp";    onActivated: { var v = curView(); if (v) v.prevPage() } }

    function _findAnnotIndex(id) {
        for (let i = 0; i < annotationStore.count; ++i) {
            if (annotationStore.data(annotationStore.index(i, 0), 257) === id)
                return i
        }
        return -1
    }
    function _annotNote(idx) {
        // NoteRole = UserRole+1 + 8 = 265
        return annotationStore.data(annotationStore.index(idx, 0), 265)
    }

    StickyEditor { id: stickyEditor }

    AnnotContextPopup {
        id: annotCtxPopup
        onEditNoteRequested: (id, sceneX, sceneY) => {
            const idx = _findAnnotIndex(id)
            if (idx < 0) return
            const note = _annotNote(idx)
            stickyEditor.openFor(id, sceneX, sceneY, note)
        }
    }

    FileDialog {
        id: exportAnnotDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Markdown (*.md)", "All files (*)"]
        defaultSuffix: "md"
        onAccepted: {
            const p = selectedFile.toString().replace("file://", "")
            annotationStore.exportMarkdown(p)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            Layout.fillWidth: true
            invertOn: root.invertColors
            leftOn:   root.leftPanelVisible
            rightOn:  root.rightPanelVisible
            onOpenRequested:        openDialog.open()
            onNewTabRequested:      openDialog.open()
            onCloseTabRequested:    documentController.closeCurrent()
            onZoomInRequested:      { var v = curView(); if (v) v.zoomIn() }
            onZoomOutRequested:     { var v = curView(); if (v) v.zoomOut() }
            onFitWidthRequested:    { var v = curView(); if (v) v.fitWidth() }
            onFitPageRequested:     { var v = curView(); if (v) v.fitPage() }
            onSearchRequested:      searchBar.openWithFocus()
            onInvertToggled:        root.invertColors = !root.invertColors
            onLeftPanelToggled:     root.leftPanelVisible = !root.leftPanelVisible
            onRightPanelToggled:    root.rightPanelVisible = !root.rightPanelVisible
        }
        TabsBar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
        }

        AnnotToolbar {
            id: annotToolbar
            Layout.fillWidth: true
            expanded: root.annotToolbarExpanded
        }

        SplitView {
            id: split
            orientation: Qt.Horizontal
            Layout.fillWidth: true
            Layout.fillHeight: true

            ThumbnailList {
                id: thumbs
                SplitView.preferredWidth: 180
                SplitView.minimumWidth: 100
                visible: root.leftPanelVisible
                document: {
                    const cur = documentModel.currentIndex
                    return cur < 0 ? null : documentModel.index(cur, 0).data(261)
                }
                currentPage: {
                    const v = curView()
                    return v ? v.currentPage : 0
                }
                onPageActivated: (p) => { var v = curView(); if (v) v.goToPage(p) }
            }

            Item {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 300

                StackLayout {
                    id: stack
                    anchors.fill: parent
                    currentIndex: documentModel.count === 0 ? 0 : (documentModel.currentIndex + 1)

                    RecentFilesView { onOpenRequested: openDialog.open() }

                    Repeater {
                        model: documentModel
                        delegate: PdfView {
                            document: model.document !== undefined ? model.document : null
                            invertColors: root.invertColors
                            searchController: root.sharedSearchController
                            activeAnnotTool: annotToolbar.activeTool
                            activeAnnotColor: annotToolbar.activeColor
                            activeInkWidth: annotToolbar.activeInkWidth
                            onTextSelected: (page, rects, scenePt) => {
                                console.log("Main.onTextSelected page=" + page
                                            + " rects=" + rects.length
                                            + " tool=" + annotToolbar.activeTool
                                            + " color=" + annotToolbar.activeColor)
                                // If Highlight tool is active, apply the chosen style directly
                                // and skip the popup.
                                if (annotToolbar.activeTool === 3) {
                                    const c = annotToolbar.activeColor
                                    switch (annotToolbar.highlightStyle) {
                                        case 0: annotationStore.addHighlight(page, rects, c); break
                                        case 1: annotationStore.addUnderline(page, rects, c); break
                                        case 2: annotationStore.addStrikeout(page, rects, c); break
                                    }
                                    return
                                }
                                // Hand tool: show type+color popup as before
                                const parentPt = selPopup.parent.mapFromItem(null, scenePt.x, scenePt.y)
                                selPopup.showAt(parentPt.x, parentPt.y, page, rects)
                            }
                            onStickyCreated: (id, page, scenePt) => {
                                const parentPt = stickyEditor.parent.mapFromItem(null, scenePt.x, scenePt.y)
                                stickyEditor.openFor(id, parentPt.x, parentPt.y, "")
                            }
                            onAnnotationClicked: (id, type, scenePt) => {
                                const parentPt = annotCtxPopup.parent.mapFromItem(null, scenePt.x, scenePt.y)
                                annotCtxPopup.showAt(id, type, parentPt.x, parentPt.y)
                            }
                        }
                    }
                }

                SelectionPopup {
                    id: selPopup
                }

                SearchBar {
                    id: searchBar
                    controller: root.sharedSearchController
                    anchors.top: parent.top
                    anchors.topMargin: 8
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClosed: searchController.clear()
                }
            }

            OutlinePanel {
                id: outline
                SplitView.preferredWidth: 240
                SplitView.minimumWidth: 120
                visible: root.rightPanelVisible
                outlineModel: root.sharedOutlineModel
                onPageActivated: (p) => { var v = curView(); if (v) v.goToPage(p) }
            }
        }
    }
}
