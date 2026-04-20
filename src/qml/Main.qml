import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quill 1.0 as Q
import HyprPDF 1.0

ApplicationWindow {
    id: root
    width: 1200
    height: 800
    visible: true
    color: Theme.base
    title: "HyprPDF"

    property string initialFile: ""

    Component.onCompleted: {
        // Bridge HyprPDF Theme singleton into Quill.Theme so Quill components
        // pick up the active Catppuccin palette.
        Q.Theme.background       = Qt.binding(function() { return Theme.base })
        Q.Theme.backgroundAlt    = Qt.binding(function() { return Theme.mantle })
        Q.Theme.backgroundDeep   = Qt.binding(function() { return Theme.crust })
        Q.Theme.surface0         = Qt.binding(function() { return Theme.surface })
        Q.Theme.surface1         = Qt.binding(function() {
            return Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10)
        })
        Q.Theme.surface2         = Qt.binding(function() {
            return Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.15)
        })
        Q.Theme.textPrimary      = Qt.binding(function() { return Theme.text })
        Q.Theme.textSecondary    = Qt.binding(function() { return Theme.subtext })
        Q.Theme.textTertiary     = Qt.binding(function() { return Theme.muted })
        Q.Theme.primary          = Qt.binding(function() { return Theme.accent })
        Q.Theme.accent           = Qt.binding(function() { return Theme.accent })
        Q.Theme.success          = Qt.binding(function() { return Theme.success })
        Q.Theme.warning          = Qt.binding(function() { return Theme.warning })
        Q.Theme.error            = Qt.binding(function() { return Theme.error })
        Q.Theme.radiusSm         = Qt.binding(function() { return Theme.radiusSmall })
        Q.Theme.radius           = Qt.binding(function() { return Theme.radiusMedium })
        Q.Theme.radiusLg         = Qt.binding(function() { return Theme.radiusLarge })
        Q.Theme.animDuration     = Qt.binding(function() { return Theme.animDuration })
        Q.Theme.animDurationFast = Qt.binding(function() { return Theme.animDurationFast })
        Q.Theme.animDurationSlow = Qt.binding(function() { return Theme.animDurationSlow })

        if (initialFile.length > 0)
            documentController.open(initialFile)
    }

    Connections {
        target: documentController
        function onOpenFailed(reason) {
            console.warn("HyprPDF open failed:", reason)
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
        var i = stack.currentIndex
        if (i <= 0) return null
        // stack.itemAt(i) — StackLayout children accessible via itemAt
        return stack.itemAt(i)
    }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem { text: "Open...";    shortcut: "Ctrl+O"; onTriggered: openDialog.open() }
            MenuItem { text: "New tab";    shortcut: "Ctrl+T"; onTriggered: openDialog.open() }
            MenuItem { text: "Close tab";  shortcut: "Ctrl+W"; onTriggered: documentController.closeCurrent() }
            MenuSeparator {}
            MenuItem { text: "Quit";       shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "&View"
            MenuItem { text: "Zoom in";   shortcut: "Ctrl++"; onTriggered: { var v = curView(); if (v) v.zoomIn() } }
            MenuItem { text: "Zoom out";  shortcut: "Ctrl+-"; onTriggered: { var v = curView(); if (v) v.zoomOut() } }
            MenuItem { text: "Fit width"; shortcut: "Ctrl+1"; onTriggered: { var v = curView(); if (v) v.fitWidth() } }
            MenuItem { text: "Fit page";  shortcut: "Ctrl+2"; onTriggered: { var v = curView(); if (v) v.fitPage() } }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            Layout.fillWidth: true
            onOpenRequested:    openDialog.open()
            onZoomInRequested:  { var v = curView(); if (v) v.zoomIn() }
            onZoomOutRequested: { var v = curView(); if (v) v.zoomOut() }
            onFitPageRequested: { var v = curView(); if (v) v.fitPage() }
        }

        TabsBar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
        }

        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            // index 0 = recent-files splash; indices 1..N = one PdfView per tab
            currentIndex: documentModel.count === 0 ? 0 : (documentModel.currentIndex + 1)

            RecentFilesView {
                onOpenRequested: openDialog.open()
            }

            Repeater {
                model: documentModel
                delegate: PdfView {
                    document: model.document !== undefined ? model.document : null
                }
            }
        }
    }

    Shortcut { sequence: "j";      onActivated: { var v = curView(); if (v) v.scrollBy(80) } }
    Shortcut { sequence: "k";      onActivated: { var v = curView(); if (v) v.scrollBy(-80) } }
    Shortcut { sequence: "PgDown"; onActivated: { var v = curView(); if (v) v.nextPage() } }
    Shortcut { sequence: "PgUp";   onActivated: { var v = curView(); if (v) v.prevPage() } }
}
