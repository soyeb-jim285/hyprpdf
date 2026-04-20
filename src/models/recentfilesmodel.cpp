#include "recentfilesmodel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDebug>

RecentFilesModel::RecentFilesModel(const QString &path, QObject *parent)
    : QAbstractListModel(parent) {
    if (path.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir().mkpath(dir);
        m_storePath = dir + "/recent.json";
    } else {
        m_storePath = path;
        QDir().mkpath(QFileInfo(path).absolutePath());
    }
    load();
}

RecentFilesModel::~RecentFilesModel() { save(); }

int RecentFilesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_entries.size();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.size()) return {};
    const auto &e = m_entries[index.row()];
    QFileInfo fi(e.path);
    switch (role) {
        case PathRole:     return e.path;
        case NameRole:     return fi.fileName();
        case SizeRole:     return fi.size();
        case ModifiedRole: return fi.lastModified();
        case IconNameRole: return iconNameFor(e.path);
        case ExistsRole:   return fi.exists();
    }
    return {};
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const {
    return {
        {PathRole,     "path"},
        {NameRole,     "name"},
        {SizeRole,     "size"},
        {ModifiedRole, "modified"},
        {IconNameRole, "iconName"},
        {ExistsRole,   "exists"}
    };
}

void RecentFilesModel::addRecent(const QString &path) {
    if (path.isEmpty()) return;
    int existing = -1;
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].path == path) { existing = i; break; }
    }
    if (existing >= 0) {
        beginRemoveRows({}, existing, existing);
        m_entries.remove(existing);
        endRemoveRows();
    }
    beginInsertRows({}, 0, 0);
    m_entries.prepend({path, QDateTime::currentDateTime()});
    endInsertRows();
    while (m_entries.size() > kMax) {
        beginRemoveRows({}, m_entries.size() - 1, m_entries.size() - 1);
        m_entries.removeLast();
        endRemoveRows();
    }
    save();
}

void RecentFilesModel::remove(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    beginRemoveRows({}, index, index);
    m_entries.remove(index);
    endRemoveRows();
    save();
}

void RecentFilesModel::clear() {
    if (m_entries.isEmpty()) return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
    save();
}

void RecentFilesModel::open(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    emit requestOpen(m_entries[index].path);
}

void RecentFilesModel::load() {
    QFile f(m_storePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return;
    beginResetModel();
    m_entries.clear();
    for (const auto &v : doc.array()) {
        const auto o = v.toObject();
        Entry e;
        e.path    = o.value("path").toString();
        e.addedAt = QDateTime::fromString(o.value("addedAt").toString(), Qt::ISODate);
        if (!e.path.isEmpty()) m_entries.append(e);
    }
    endResetModel();
}

void RecentFilesModel::save() {
    QJsonArray arr;
    for (const auto &e : m_entries) {
        arr.append(QJsonObject{
            {"path", e.path},
            {"addedAt", e.addedAt.toString(Qt::ISODate)}
        });
    }
    QSaveFile f(m_storePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "RecentFilesModel: cannot write" << m_storePath << f.errorString();
        return;
    }
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.commit();
}

QString RecentFilesModel::iconNameFor(const QString &path) {
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "pdf") return "IconFileText";
    if (ext == "pptx" || ext == "ppt" || ext == "odp") return "IconLayoutGrid";
    if (ext == "docx" || ext == "doc"  || ext == "odt") return "IconFileText";
    if (ext == "epub" || ext == "md"   || ext == "html") return "IconBookOpen";
    return "IconFileText";
}
