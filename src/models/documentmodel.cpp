#include "documentmodel.h"
#include "services/pdfdoc.h"
#include <QFileInfo>
#include <QDebug>

DocumentModel::DocumentModel(QObject *parent) : QAbstractListModel(parent) {}

DocumentModel::~DocumentModel() {
    qDeleteAll(m_tabs);
    m_tabs.clear();
}

int DocumentModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_tabs.size();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_tabs.size()) return {};
    const Tab *t = m_tabs[index.row()];
    switch (role) {
        case PathRole:      return t->path;
        case TitleRole:     return t->title;
        case PageCountRole: return t->doc ? t->doc->pageCount() : 0;
        case IsDirtyRole:   return t->dirty;
        case DocumentRole:  return QVariant::fromValue<QObject*>(t->doc.get());
    }
    return {};
}

QHash<int, QByteArray> DocumentModel::roleNames() const {
    return {
        {PathRole,      "path"},
        {TitleRole,     "title"},
        {PageCountRole, "pageCount"},
        {IsDirtyRole,   "dirty"},
        {DocumentRole,  "document"}
    };
}

void DocumentModel::setCurrentIndex(int idx) {
    if (m_tabs.isEmpty()) {
        if (m_current != -1) { m_current = -1; emit currentIndexChanged(); }
        return;
    }
    const int clamped = qBound(0, idx, m_tabs.size() - 1);
    if (clamped == m_current) return;
    m_current = clamped;
    emit currentIndexChanged();
}

int DocumentModel::openDocument(const QString &path) {
    if (path.isEmpty()) {
        emit openFailed(path, "empty path");
        return -1;
    }
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i]->path == path) {
            setCurrentIndex(i);
            return i;
        }
    }
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable()) {
        emit openFailed(path, "file not found or unreadable");
        return -1;
    }
    auto doc = PdfDoc::load(path);
    if (!doc) {
        emit openFailed(path, "Poppler could not load the document");
        return -1;
    }
    QString title = doc->title();
    if (title.isEmpty()) title = fi.fileName();
    auto *t = new Tab{path, title, std::move(doc), false};
    beginInsertRows({}, m_tabs.size(), m_tabs.size());
    m_tabs.append(t);
    endInsertRows();
    emit countChanged();
    setCurrentIndex(m_tabs.size() - 1);
    return m_tabs.size() - 1;
}

void DocumentModel::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size()) return;
    beginRemoveRows({}, index, index);
    delete m_tabs.takeAt(index);
    endRemoveRows();
    emit countChanged();
    if (m_tabs.isEmpty()) {
        m_current = -1;
        emit currentIndexChanged();
    } else {
        setCurrentIndex(qMin(m_current, m_tabs.size() - 1));
    }
}
