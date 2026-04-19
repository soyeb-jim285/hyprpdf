import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1200
    height: 800
    visible: true
    title: pdfView.documentTitle.length > 0
           ? pdfView.documentTitle + " — HyprPDF"
           : "HyprPDF"

    property string initialFile: ""

    Component.onCompleted: {
        if (initialFile.length > 0)
            pdfView.openPath(initialFile)
    }

    FileDialog {
        id: openDialog
        title: "Open document"
        nameFilters: [
            "Documents (*.pdf *.pptx *.ppt *.odp *.docx *.doc *.odt *.rtf *.epub *.md *.html)",
            "PDF (*.pdf)",
            "Presentations (*.pptx *.ppt *.odp)",
            "Text documents (*.docx *.doc *.odt *.rtf)",
            "All files (*)"
        ]
        onAccepted: pdfView.openPath(selectedFile.toString().replace("file://", ""))
    }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem { text: "Open…"; shortcut: "Ctrl+O"; onTriggered: openDialog.open() }
            MenuItem { text: "Close tab"; shortcut: "Ctrl+W" }
            MenuSeparator {}
            MenuItem { text: "Quit"; shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "&View"
            MenuItem { text: "Zoom in"; shortcut: "Ctrl++"; onTriggered: pdfView.zoomIn() }
            MenuItem { text: "Zoom out"; shortcut: "Ctrl+-"; onTriggered: pdfView.zoomOut() }
            MenuItem { text: "Fit width"; shortcut: "Ctrl+1"; onTriggered: pdfView.fitWidth() }
            MenuItem { text: "Fit page"; shortcut: "Ctrl+2"; onTriggered: pdfView.fitPage() }
        }
    }

    PdfView {
        id: pdfView
        anchors.fill: parent
    }

    Shortcut { sequence: "Ctrl+O"; onActivated: openDialog.open() }
    Shortcut { sequence: "j"; onActivated: pdfView.scrollBy(80) }
    Shortcut { sequence: "k"; onActivated: pdfView.scrollBy(-80) }
    Shortcut { sequence: "PgDown"; onActivated: pdfView.nextPage() }
    Shortcut { sequence: "PgUp"; onActivated: pdfView.prevPage() }
}
