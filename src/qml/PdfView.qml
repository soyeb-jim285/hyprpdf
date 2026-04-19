import QtQuick
import QtQuick.Controls
import QtQuick.Pdf

Item {
    id: root

    property alias document: pdfDoc
    property string documentTitle: pdfDoc.status === PdfDocument.Ready ? pdfDoc.title : ""

    function openPath(path) {
        pdfDoc.source = "file://" + path
    }
    function zoomIn()   { view.renderScale *= 1.2 }
    function zoomOut()  { view.renderScale /= 1.2 }
    function fitWidth() { view.renderScale = width / Math.max(1, pdfDoc.maxPageWidth) }
    function fitPage() {
        const s1 = width  / Math.max(1, pdfDoc.maxPageWidth)
        const s2 = height / Math.max(1, pdfDoc.maxPageHeight)
        view.renderScale = Math.min(s1, s2)
    }
    function scrollBy(dy)  { view.contentY = Math.max(0, view.contentY + dy) }
    function nextPage()    { view.goToPage(Math.min(pdfDoc.pageCount - 1, view.currentPage + 1)) }
    function prevPage()    { view.goToPage(Math.max(0, view.currentPage - 1)) }

    PdfDocument { id: pdfDoc }

    PdfMultiPageView {
        id: view
        anchors.fill: parent
        document: pdfDoc
    }

    Label {
        anchors.centerIn: parent
        visible: pdfDoc.status !== PdfDocument.Ready
        text: pdfDoc.status === PdfDocument.Null
              ? "Open a document with Ctrl+O"
              : "Loading…"
        opacity: 0.6
    }
}
