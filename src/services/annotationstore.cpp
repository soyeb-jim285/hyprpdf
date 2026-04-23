#include "annotationstore.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QUuid>
#include <QDateTime>
#include <QDebug>

static QVector<QRectF> rectsFromVariant(const QVariantList &vs) {
    QVector<QRectF> out; out.reserve(vs.size());
    for (const auto &v : vs) out.append(v.toRectF());
    return out;
}

static QVector<QVector<QPointF>> strokesFromVariant(const QVariantList &vs) {
    QVector<QVector<QPointF>> out; out.reserve(vs.size());
    for (const auto &v : vs) {
        QVector<QPointF> pts;
        const auto inner = v.toList();
        for (const auto &p : inner) pts.append(p.toPointF());
        out.append(pts);
    }
    return out;
}

static QJsonArray rectsToJson(const QVector<QRectF> &rects) {
    QJsonArray a;
    for (const auto &r : rects) {
        a.append(QJsonArray{r.x(), r.y(), r.width(), r.height()});
    }
    return a;
}

static QVector<QRectF> rectsFromJson(const QJsonArray &a) {
    QVector<QRectF> out;
    for (const auto &v : a) {
        const auto row = v.toArray();
        if (row.size() == 4)
            out.append(QRectF(row[0].toDouble(), row[1].toDouble(),
                              row[2].toDouble(), row[3].toDouble()));
    }
    return out;
}

static QJsonArray strokesToJson(const QVector<QVector<QPointF>> &strokes) {
    QJsonArray a;
    for (const auto &s : strokes) {
        QJsonArray pts;
        for (const auto &p : s) pts.append(QJsonArray{p.x(), p.y()});
        a.append(pts);
    }
    return a;
}

static QVector<QVector<QPointF>> strokesFromJson(const QJsonArray &a) {
    QVector<QVector<QPointF>> out;
    for (const auto &s : a) {
        QVector<QPointF> pts;
        const auto arr = s.toArray();
        for (const auto &pv : arr) {
            const auto row = pv.toArray();
            if (row.size() == 2) pts.append(QPointF(row[0].toDouble(), row[1].toDouble()));
        }
        out.append(pts);
    }
    return out;
}

static const char *typeToString(int t) {
    switch (t) {
        case AnnotationStore::Highlight:  return "highlight";
        case AnnotationStore::Underline:  return "underline";
        case AnnotationStore::Strikeout:  return "strikeout";
        case AnnotationStore::StickyNote: return "stickyNote";
        case AnnotationStore::Ink:        return "ink";
        case AnnotationStore::TextBox:    return "textBox";
    }
    return "highlight";
}

static int typeFromString(const QString &s) {
    if (s == "underline")  return AnnotationStore::Underline;
    if (s == "strikeout")  return AnnotationStore::Strikeout;
    if (s == "stickyNote") return AnnotationStore::StickyNote;
    if (s == "ink")        return AnnotationStore::Ink;
    if (s == "textBox")    return AnnotationStore::TextBox;
    return AnnotationStore::Highlight;
}

AnnotationStore::AnnotationStore(QObject *parent) : QAbstractListModel(parent) {
    const QString dataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation) + "/annotations";
    m_storageDir = dataDir;
    QDir().mkpath(m_storageDir);
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(300);
    connect(&m_saveTimer, &QTimer::timeout, this, &AnnotationStore::save);
}

AnnotationStore::~AnnotationStore() {
    if (m_saveTimer.isActive()) save();
}

void AnnotationStore::setStorageDir(const QString &dir) {
    m_storageDir = dir;
    QDir().mkpath(m_storageDir);
}

QString AnnotationStore::sidecarPath() const {
    if (m_docHash.isEmpty()) return {};
    return m_storageDir + "/" + m_docHash + ".json";
}

int AnnotationStore::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_annots.size();
}

QHash<int, QByteArray> AnnotationStore::roleNames() const {
    return {
        {IdRole,             "id"},
        {TypeRole,           "type"},
        {PageRole,           "page"},
        {RectsRole,          "rects"},
        {AnchorRole,         "anchor"},
        {InkStrokesRole,     "strokes"},
        {InkStrokeWidthRole, "strokeWidth"},
        {ColorRole,          "color"},
        {NoteRole,           "note"},
        {AuthorRole,         "author"},
        {CreatedAtRole,      "createdAt"},
        {SelectedRole,       "selected"}
    };
}

QVariant AnnotationStore::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_annots.size()) return {};
    const Annot &a = m_annots[index.row()];
    switch (role) {
        case IdRole:        return a.id;
        case TypeRole:      return a.type;
        case PageRole:      return a.page;
        case RectsRole: {
            QVariantList out;
            for (const auto &r : a.rects) out.append(QVariant::fromValue(r));
            return out;
        }
        case AnchorRole:    return a.anchor;
        case InkStrokesRole: {
            QVariantList out;
            for (const auto &s : a.strokes) {
                QVariantList pts;
                for (const auto &p : s) pts.append(p);
                out.append(pts);
            }
            return out;
        }
        case InkStrokeWidthRole: return a.strokeWidth;
        case ColorRole:     return a.color;
        case NoteRole:      return a.note;
        case AuthorRole:    return a.author;
        case CreatedAtRole: return a.createdAt;
        case SelectedRole:  return a.id == m_selectedId;
    }
    return {};
}

void AnnotationStore::loadDocument(const QString &hash) {
    beginResetModel();
    m_annots.clear();
    m_undo.clear();
    m_redo.clear();
    m_selectedId.clear();
    m_docHash = hash;
    endResetModel();
    emit countChanged();
    emit historyChanged();
    emit selectedIdChanged();
    bumpRevision();

    if (hash.isEmpty()) return;
    const QString p = sidecarPath();
    QFile f(p);
    if (!f.open(QIODevice::ReadOnly)) return;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return;
    const auto root = doc.object();
    const auto arr = root.value("annotations").toArray();

    beginResetModel();
    for (const auto &v : arr) {
        const auto o = v.toObject();
        Annot a;
        a.id        = o.value("id").toString();
        a.type      = typeFromString(o.value("type").toString());
        a.page      = o.value("page").toInt();
        a.rects     = rectsFromJson(o.value("rects").toArray());
        const auto anc = o.value("anchor").toArray();
        if (anc.size() == 2) a.anchor = QPointF(anc[0].toDouble(), anc[1].toDouble());
        a.strokes   = strokesFromJson(o.value("strokes").toArray());
        a.strokeWidth = o.value("strokeWidth").toDouble(2.0);
        a.fontSize = o.value("fontSize").toDouble(14.0);
        a.color     = QColor(o.value("color").toString());
        a.note      = o.value("note").toString();
        a.author    = o.value("author").toString();
        a.createdAt = QDateTime::fromString(o.value("createdAt").toString(), Qt::ISODate);
        if (a.id.isEmpty()) continue;
        m_annots.append(a);
    }
    endResetModel();
    emit countChanged();
    bumpRevision();
}

void AnnotationStore::clear() {
    beginResetModel();
    m_annots.clear();
    m_undo.clear();
    m_redo.clear();
    m_selectedId.clear();
    endResetModel();
    emit countChanged();
    emit historyChanged();
    emit selectedIdChanged();
    bumpRevision();
}

void AnnotationStore::save() {
    if (m_docHash.isEmpty()) return;
    const QString p = sidecarPath();
    QDir().mkpath(QFileInfo(p).absolutePath());
    QJsonArray arr;
    for (const auto &a : m_annots) {
        QJsonObject o;
        o["id"]          = a.id;
        o["type"]        = typeToString(a.type);
        o["page"]        = a.page;
        o["rects"]       = rectsToJson(a.rects);
        o["anchor"]      = QJsonArray{a.anchor.x(), a.anchor.y()};
        o["strokes"]     = strokesToJson(a.strokes);
        o["strokeWidth"] = a.strokeWidth;
        o["fontSize"]    = a.fontSize;
        o["color"]       = a.color.name(QColor::HexRgb);
        o["note"]        = a.note;
        o["author"]      = a.author;
        o["createdAt"]   = a.createdAt.toString(Qt::ISODate);
        arr.append(o);
    }
    QJsonObject root;
    root["version"]      = 1;
    root["docHash"]      = m_docHash;
    root["annotations"]  = arr;

    QSaveFile f(p);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit saveFailed(f.errorString());
        return;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!f.commit()) emit saveFailed("commit failed");
}

int AnnotationStore::findIndex(const QString &id) const {
    for (int i = 0; i < m_annots.size(); ++i) if (m_annots[i].id == id) return i;
    return -1;
}

void AnnotationStore::scheduleSave() {
    if (!m_saveTimer.isActive()) m_saveTimer.start();
}

void AnnotationStore::bumpRevision() {
    ++m_revision;
    emit revisionChanged();
}

void AnnotationStore::pushUndo(Command c) {
    if (m_suppressHistory) return;
    m_undo.push(c);
    while (m_undo.size() > kUndoDepth) m_undo.remove(0);
    m_redo.clear();
    emit historyChanged();
}

QString AnnotationStore::addAnnotInternal(Annot a) {
    a.id = QUuid::createUuid().toString();
    a.author = m_author;
    a.createdAt = QDateTime::currentDateTimeUtc();

    beginInsertRows({}, m_annots.size(), m_annots.size());
    m_annots.append(a);
    endInsertRows();

    Command cmd{CmdAdd, Annot{}, a};
    pushUndo(cmd);

    emit countChanged();
    bumpRevision();
    scheduleSave();
    return a.id;
}

QString AnnotationStore::addHighlight(int page, QVariantList rects, const QColor &color) {
    Annot a; a.type = Highlight; a.page = page;
    a.rects = rectsFromVariant(rects); a.color = color;
    return addAnnotInternal(a);
}

QString AnnotationStore::addUnderline(int page, QVariantList rects, const QColor &color) {
    Annot a; a.type = Underline; a.page = page;
    a.rects = rectsFromVariant(rects); a.color = color;
    return addAnnotInternal(a);
}

QString AnnotationStore::addStrikeout(int page, QVariantList rects, const QColor &color) {
    Annot a; a.type = Strikeout; a.page = page;
    a.rects = rectsFromVariant(rects); a.color = color;
    return addAnnotInternal(a);
}

QString AnnotationStore::addStickyNote(int page, QPointF anchor,
                                       const QColor &color, const QString &note) {
    Annot a; a.type = StickyNote; a.page = page;
    a.anchor = anchor; a.color = color; a.note = note;
    return addAnnotInternal(a);
}

QString AnnotationStore::addInk(int page, QVariantList strokes,
                                 const QColor &color, qreal strokeWidth) {
    Annot a; a.type = Ink; a.page = page;
    a.strokes = strokesFromVariant(strokes);
    a.color = color; a.strokeWidth = strokeWidth;
    return addAnnotInternal(a);
}

QString AnnotationStore::addTextBox(int page, QPointF anchor,
                                     const QColor &color, const QString &text,
                                     qreal fontSize) {
    Annot a; a.type = TextBox; a.page = page;
    a.anchor = anchor; a.color = color; a.note = text; a.fontSize = fontSize;
    return addAnnotInternal(a);
}

void AnnotationStore::remove(const QString &id) {
    const int idx = findIndex(id);
    if (idx < 0) return;
    const Annot before = m_annots[idx];
    beginRemoveRows({}, idx, idx);
    m_annots.remove(idx);
    endRemoveRows();
    pushUndo(Command{CmdRemove, before, Annot{}});
    emit countChanged();
    bumpRevision();
    scheduleSave();
}

void AnnotationStore::setColor(const QString &id, const QColor &color) {
    const int idx = findIndex(id);
    if (idx < 0) return;
    const Annot before = m_annots[idx];
    m_annots[idx].color = color;
    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {ColorRole});
    pushUndo(Command{CmdSetColor, before, m_annots[idx]});
    bumpRevision();
    scheduleSave();
}

void AnnotationStore::setNote(const QString &id, const QString &note) {
    const int idx = findIndex(id);
    if (idx < 0) return;
    const Annot before = m_annots[idx];
    m_annots[idx].note = note;
    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {NoteRole});
    pushUndo(Command{CmdSetNote, before, m_annots[idx]});
    bumpRevision();
    scheduleSave();
}

QVariantList AnnotationStore::annotationsOnPage(int page) const {
    QVariantList out;
    for (int i = 0; i < m_annots.size(); ++i) {
        if (m_annots[i].page != page) continue;
        QVariantMap m;
        m["id"]          = m_annots[i].id;
        m["type"]        = m_annots[i].type;
        m["page"]        = m_annots[i].page;
        QVariantList rects;
        for (const auto &r : m_annots[i].rects) rects.append(QVariant::fromValue(r));
        m["rects"]       = rects;
        m["anchor"]      = m_annots[i].anchor;
        QVariantList strokes;
        for (const auto &s : m_annots[i].strokes) {
            QVariantList pts;
            for (const auto &p : s) pts.append(p);
            strokes.append(pts);
        }
        m["strokes"]     = strokes;
        m["strokeWidth"] = m_annots[i].strokeWidth;
        m["fontSize"]    = m_annots[i].fontSize;
        m["color"]       = m_annots[i].color;
        m["note"]        = m_annots[i].note;
        out.append(m);
    }
    return out;
}

void AnnotationStore::applyCommandForward(const Command &c) {
    m_suppressHistory = true;
    switch (c.kind) {
        case CmdAdd: {
            beginInsertRows({}, m_annots.size(), m_annots.size());
            m_annots.append(c.after);
            endInsertRows();
            break;
        }
        case CmdRemove: {
            const int idx = findIndex(c.before.id);
            if (idx >= 0) {
                beginRemoveRows({}, idx, idx);
                m_annots.remove(idx);
                endRemoveRows();
            }
            break;
        }
        case CmdSetColor:
        case CmdSetNote: {
            const int idx = findIndex(c.after.id);
            if (idx >= 0) {
                m_annots[idx] = c.after;
                const QModelIndex mi = index(idx);
                emit dataChanged(mi, mi);
            }
            break;
        }
    }
    m_suppressHistory = false;
}

void AnnotationStore::applyCommandBackward(const Command &c) {
    m_suppressHistory = true;
    switch (c.kind) {
        case CmdAdd: {
            const int idx = findIndex(c.after.id);
            if (idx >= 0) {
                beginRemoveRows({}, idx, idx);
                m_annots.remove(idx);
                endRemoveRows();
            }
            break;
        }
        case CmdRemove: {
            beginInsertRows({}, m_annots.size(), m_annots.size());
            m_annots.append(c.before);
            endInsertRows();
            break;
        }
        case CmdSetColor:
        case CmdSetNote: {
            const int idx = findIndex(c.before.id);
            if (idx >= 0) {
                m_annots[idx] = c.before;
                const QModelIndex mi = index(idx);
                emit dataChanged(mi, mi);
            }
            break;
        }
    }
    m_suppressHistory = false;
}

void AnnotationStore::undo() {
    if (m_undo.isEmpty()) return;
    const Command c = m_undo.pop();
    applyCommandBackward(c);
    m_redo.push(c);
    emit countChanged();
    emit historyChanged();
    bumpRevision();
    scheduleSave();
}

void AnnotationStore::redo() {
    if (m_redo.isEmpty()) return;
    const Command c = m_redo.pop();
    applyCommandForward(c);
    m_undo.push(c);
    emit countChanged();
    emit historyChanged();
    bumpRevision();
    scheduleSave();
}

void AnnotationStore::setSelectedId(const QString &id) {
    if (id == m_selectedId) return;
    m_selectedId = id;
    emit selectedIdChanged();
    bumpRevision();
}

void AnnotationStore::setCurrentAuthor(const QString &a) {
    if (a == m_author) return;
    m_author = a;
    emit currentAuthorChanged();
}

void AnnotationStore::exportMarkdown(const QString &path) const {
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "exportMarkdown: cannot open" << path << f.errorString();
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);

    out << "# Annotations\n\n";
    out << "SHA-256: `" << m_docHash << "`\n";
    out << "Exported: " << QDateTime::currentDateTimeUtc().toString(Qt::ISODate) << "\n";
    out << "Author: " << m_author << "\n\n";

    QVector<Annot> sorted = m_annots;
    std::sort(sorted.begin(), sorted.end(), [](const Annot &a, const Annot &b) {
        if (a.page != b.page) return a.page < b.page;
        return a.createdAt < b.createdAt;
    });

    int curPage = -1;
    for (const auto &a : sorted) {
        if (a.page != curPage) {
            curPage = a.page;
            out << "## Page " << (curPage + 1) << "\n\n";
        }
        const QString colorHex = a.color.name(QColor::HexRgb);
        switch (a.type) {
            case Highlight:
                out << "- **Highlight** `" << colorHex << "`\n";
                break;
            case Underline:
                out << "- **Underline** `" << colorHex << "`\n";
                break;
            case Strikeout:
                out << "- **Strikeout** `" << colorHex << "`\n";
                break;
            case StickyNote:
                out << "- **Note** `" << colorHex << "` at ("
                    << int(a.anchor.x()) << ", " << int(a.anchor.y())
                    << ") — " << a.note << "\n";
                break;
            case Ink:
                out << "- **Ink** `" << colorHex << "` ("
                    << a.strokes.size() << " strokes)\n";
                break;
            case TextBox:
                out << "- **Text** `" << colorHex << "` at ("
                    << int(a.anchor.x()) << ", " << int(a.anchor.y())
                    << ") — " << a.note << "\n";
                break;
        }
    }
    out.flush();
    f.commit();
}
