#include "searchcontroller.h"
#include "pdfdoc.h"
#include <QDebug>

SearchController::SearchController(QObject *parent) : QObject(parent) {}

int SearchController::currentPage() const {
    if (m_current < 0 || m_current >= m_matches.size()) return -1;
    return m_matches[m_current].page;
}

QRectF SearchController::currentRect() const {
    if (m_current < 0 || m_current >= m_matches.size()) return {};
    return m_matches[m_current].rect;
}

void SearchController::setDocument(QObject *doc) {
    m_doc = qobject_cast<PdfDoc*>(doc);
    clear();
}

void SearchController::search(const QString &q) {
    qDebug() << "SearchController::search q=" << q
             << "doc=" << (m_doc ? "ok" : "null")
             << "pages=" << (m_doc ? m_doc->pageCount() : 0);
    m_query = q;
    emit queryChanged();
    m_matches.clear();
    m_current = -1;
    if (!m_doc || q.isEmpty()) {
        ++m_revision; emit resultsChanged();
        emit currentChanged();
        return;
    }
    for (int p = 0; p < m_doc->pageCount(); ++p) {
        const auto rects = m_doc->search(p, q);
        for (const auto &v : rects) {
            m_matches.push_back({p, v.toRectF()});
        }
    }
    qDebug() << "SearchController::search found" << m_matches.size() << "matches";
    if (!m_matches.isEmpty()) m_current = 0;
    ++m_revision; emit resultsChanged();
    emitCurrent();
}

void SearchController::next() {
    if (m_matches.isEmpty()) return;
    m_current = (m_current + 1) % m_matches.size();
    emitCurrent();
}

void SearchController::prev() {
    if (m_matches.isEmpty()) return;
    m_current = (m_current - 1 + m_matches.size()) % m_matches.size();
    emitCurrent();
}

void SearchController::clear() {
    if (m_matches.isEmpty() && m_query.isEmpty()) return;
    m_query.clear();
    m_matches.clear();
    m_current = -1;
    emit queryChanged();
    ++m_revision; emit resultsChanged();
    emit currentChanged();
}

QVariantList SearchController::matchesOnPage(int page) const {
    QVariantList out;
    for (const auto &m : m_matches) {
        if (m.page == page) out.append(QVariant::fromValue(m.rect));
    }
    return out;
}

void SearchController::emitCurrent() {
    emit currentChanged();
    if (m_current >= 0) emit scrollToRequested(currentPage(), currentRect());
}
