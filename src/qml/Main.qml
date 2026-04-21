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

    Shortcut { sequence: "/";       onActivated: searchBar.openWithFocus() }
    Shortcut { sequence: "Escape";  onActivated: { if (searchBar.visible) { searchBar.visible = false; searchController.clear() } } }
    Shortcut { sequence: "g,g";     onActivated: { var v = curView(); if (v) v.goToPage(0) } }
    Shortcut { sequence: "Shift+G"; onActivated: { var v = curView(); if (v && v.document) v.goToPage(v.document.pageCount - 1) } }
    Shortcut { sequence: "j";       onActivated: { var v = curView(); if (v) v.scrollBy(80) } }
    Shortcut { sequence: "k";       onActivated: { var v = curView(); if (v) v.scrollBy(-80) } }
    Shortcut { sequence: "PgDown";  onActivated: { var v = curView(); if (v) v.nextPage() } }
    Shortcut { sequence: "PgUp";    onActivated: { var v = curView(); if (v) v.prevPage() } }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem { action: actOpen }
            MenuItem { action: actNewTab }
            MenuItem { action: actCloseTab }
            MenuSeparator {}
            MenuItem { action: actQuit }
        }
        Menu {
            title: "&View"
            MenuItem { action: actZoomIn }
            MenuItem { action: actZoomOut }
            MenuItem { action: actFitWidth }
            MenuItem { action: actFitPage }
            MenuSeparator {}
            MenuItem { action: actInvert }
            MenuItem { action: actLeft }
            MenuItem { action: actRight }
        }
        Menu {
            title: "&Find"
            MenuItem { action: actFind }
            MenuItem { action: actFindNext }
            MenuItem { action: actFindPrev }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
            onZoomInRequested:  { var v = curView(); if (v) v.zoomIn() }
            onZoomOutRequested: { var v = curView(); if (v) v.zoomOut() }
            onFitPageRequested: { var v = curView(); if (v) v.fitPage() }
        }
        TabsBar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
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
                            searchController: searchController
                        }
                    }
                }

                SearchBar {
                    id: searchBar
                    controller: searchController
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
                outlineModel: outlineModel
                onPageActivated: (p) => { var v = curView(); if (v) v.goToPage(p) }
            }
        }
    }
}
