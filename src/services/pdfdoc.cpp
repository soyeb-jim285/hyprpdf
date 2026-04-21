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

QVariantList PdfDoc::search(int page, const QString &text) const {
    QVariantList result;
    if (!m_doc || page < 0 || page >= m_doc->numPages() || text.isEmpty()) return result;
    auto p = m_doc->page(page);
    if (!p) return result;
    auto rects = p->search(text, Poppler::Page::IgnoreCase);
    for (const auto &r : rects) result.append(QVariant::fromValue(r));
    return result;
}

QString PdfDoc::textInRect(int page, const QRectF &rect) const {
    if (!m_doc || page < 0 || page >= m_doc->numPages() || rect.isEmpty()) return {};
    auto p = m_doc->page(page);
    if (!p) return {};
    return p->text(rect).trimmed();
}

QVariantList PdfDoc::selectionRects(int page, const QRectF &rect) const {
    QVariantList result;
    if (!m_doc || page < 0 || page >= m_doc->numPages() || rect.isEmpty()) return result;
    auto p = m_doc->page(page);
    if (!p) return result;
    // textList() returns all word boxes on the page (PDF points, top-left origin).
    auto words = p->textList();
    // Line-aware selection: any word whose vertical span intersects the
    // drag's vertical span is considered a candidate; then cull on horizontal
    // span so top/bottom partial rows only include the horizontally-hit words.
    const double yTop = rect.top();
    const double yBot = rect.bottom();
    const double xLeft = rect.left();
    const double xRight = rect.right();
    for (const auto &w : words) {
        const QRectF bb = w->boundingBox();
        // Vertical intersect test: word row overlaps drag vertical span
        if (bb.bottom() < yTop || bb.top() > yBot) continue;
        // For words fully inside vertical span (mid-rows), keep regardless of x
        const bool topRow = bb.bottom() >= yTop && bb.top() < yTop;
        const bool botRow = bb.top() <= yBot && bb.bottom() > yBot;
        if (topRow || botRow) {
            // Border row — require horizontal overlap with drag rect
            if (bb.right() < xLeft || bb.left() > xRight) continue;
        }
        result.append(QVariant::fromValue(bb));
    }
    return result;
}

static void buildTocFromOutline(const QVector<Poppler::OutlineItem> &items,
                                QVector<PdfDoc::TocEntry> &out) {
    for (const auto &item : items) {
        PdfDoc::TocEntry e;
        e.title = item.name();
        auto dest = item.destination();
        e.pageIndex = (dest && dest->pageNumber() > 0) ? dest->pageNumber() - 1 : -1;
        buildTocFromOutline(item.children(), e.children);
        out.append(e);
    }
}

const QVector<PdfDoc::TocEntry> &PdfDoc::toc() const {
    if (!m_tocBuilt) {
        m_tocBuilt = true;
        if (m_doc) buildTocFromOutline(m_doc->outline(), m_toc);
    }
    return m_toc;
}
