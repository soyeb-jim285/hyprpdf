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
    for (const auto &r : rects) result.append(QVariant::fromValue(r.normalized()));
    return result;
}

QString PdfDoc::textInRect(int page, const QRectF &rect) const {
    if (!m_doc || page < 0 || page >= m_doc->numPages() || rect.isEmpty()) return {};
    auto p = m_doc->page(page);
    if (!p) return {};
    return p->text(rect).trimmed();
}

QVariantList PdfDoc::selectionRectsLine(int page, qreal sx, qreal sy,
                                         qreal ex, qreal ey) const {
    QVariantList result;
    if (!m_doc || page < 0 || page >= m_doc->numPages()) return result;
    auto p = m_doc->page(page);
    if (!p) return result;
    auto words = p->textList();
    if (words.empty()) return result;

    // Flatten to character-level bboxes so selection can split words.
    struct Glyph { QRectF bb; int line = -1; };
    std::vector<Glyph> glyphs;
    glyphs.reserve(words.size() * 6);
    for (const auto &w : words) {
        const QRectF wordBB = w->boundingBox();
        const int n = static_cast<int>(w->text().size());
        if (n <= 0) { glyphs.push_back({wordBB}); continue; }
        bool anyChar = false;
        for (int i = 0; i < n; ++i) {
            const QRectF cbb = w->charBoundingBox(i);
            if (cbb.isValid() && !cbb.isEmpty()) {
                glyphs.push_back({cbb});
                anyChar = true;
            }
        }
        if (!anyChar) glyphs.push_back({wordBB});
    }
    if (glyphs.empty()) return result;

    // Cluster by y-center into lines.
    double avgH = 0;
    for (const auto &g : glyphs) avgH += g.bb.height();
    avgH /= std::max<size_t>(1, glyphs.size());
    const double lineTol = std::max(1.0, avgH * 0.5);

    std::sort(glyphs.begin(), glyphs.end(), [](const Glyph &a, const Glyph &b) {
        const double ay = a.bb.center().y(), by = b.bb.center().y();
        if (std::abs(ay - by) < 0.001) return a.bb.left() < b.bb.left();
        return ay < by;
    });
    int curLine = 0;
    double curY = glyphs.front().bb.center().y();
    for (auto &g : glyphs) {
        const double y = g.bb.center().y();
        if (std::abs(y - curY) > lineTol) {
            ++curLine;
            curY = y;
        }
        g.line = curLine;
    }

    auto pointLine = [&](double y) -> int {
        for (const auto &g : glyphs) {
            if (g.bb.top() <= y && g.bb.bottom() >= y) return g.line;
        }
        if (y <= glyphs.front().bb.top()) return glyphs.front().line;
        return glyphs.back().line;
    };
    int lineS = pointLine(sy);
    int lineE = pointLine(ey);
    if (lineS > lineE || (lineS == lineE && sx > ex)) {
        std::swap(sx, ex);
        std::swap(sy, ey);
        std::swap(lineS, lineE);
    }

    // Collect selected glyphs then union per-line into one rect per line span.
    std::vector<QRectF> unionPerLine(curLine + 1);
    std::vector<bool> lineHasSel(curLine + 1, false);
    for (const auto &g : glyphs) {
        if (g.line < lineS || g.line > lineE) continue;
        const double cx = g.bb.center().x();
        if (lineS == lineE) {
            const double lo = std::min(sx, ex), hi = std::max(sx, ex);
            if (cx < lo || cx > hi) continue;
        } else if (g.line == lineS) {
            if (cx < sx) continue;
        } else if (g.line == lineE) {
            if (cx > ex) continue;
        }
        if (!lineHasSel[g.line]) {
            unionPerLine[g.line] = g.bb;
            lineHasSel[g.line] = true;
        } else {
            unionPerLine[g.line] = unionPerLine[g.line].united(g.bb);
        }
    }
    for (int i = lineS; i <= lineE; ++i) {
        if (lineHasSel[i]) result.append(QVariant::fromValue(unionPerLine[i]));
    }
    return result;
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
