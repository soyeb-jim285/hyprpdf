#include "pdfdoc.h"
// NOTE: include path uses -I/usr/include/poppler/qt6; header lives at poppler/qt6/poppler-qt6.h
#include <poppler-qt6.h>
#include <QDebug>

int PdfDoc::s_nextId = 1;
QHash<int, PdfDoc*> PdfDoc::s_registry;

std::unique_ptr<PdfDoc> PdfDoc::load(const QString &path, QObject *parent) {
    auto doc = Poppler::Document::load(path);
    if (!doc || doc->isLocked()) return nullptr;
    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);
    doc->setRenderHint(Poppler::Document::TextHinting, true);
    return std::unique_ptr<PdfDoc>(new PdfDoc(std::move(doc), parent));
}

PdfDoc *PdfDoc::byId(int id) {
    return s_registry.value(id, nullptr);
}

PdfDoc::PdfDoc(std::unique_ptr<Poppler::Document> doc, QObject *parent)
    : QObject(parent), m_id(s_nextId++), m_doc(std::move(doc)) {
    s_registry.insert(m_id, this);
}

PdfDoc::~PdfDoc() {
    s_registry.remove(m_id);
}

bool PdfDoc::isValid() const {
    return m_doc != nullptr;
}

int PdfDoc::pageCount() const {
    return m_doc ? m_doc->numPages() : 0;
}

QString PdfDoc::title() const {
    if (!m_doc) return {};
    return m_doc->title();
}

QSizeF PdfDoc::pageSize(int index) const {
    if (!m_doc || index < 0 || index >= m_doc->numPages()) return {};
    auto p = m_doc->page(index);
    // pageSizeF() returns QSizeF in points (1/72 inch); pageSize() returns QSize (integer)
    return p ? p->pageSizeF() : QSizeF{};
}

QString PdfDoc::text(int index) const {
    if (!m_doc || index < 0 || index >= m_doc->numPages()) return {};
    auto p = m_doc->page(index);
    return p ? p->text({}) : QString{};
}

QImage PdfDoc::renderPage(int index, int targetWidth) const {
    if (!m_doc || index < 0 || index >= m_doc->numPages() || targetWidth <= 0) return {};
    auto p = m_doc->page(index);
    if (!p) return {};
    const QSizeF ps = p->pageSizeF();   // points (1/72 inch)
    if (ps.width() <= 0) return {};
    const double dpi = 72.0 * (static_cast<double>(targetWidth) / ps.width());
    return p->renderToImage(dpi, dpi);
}
