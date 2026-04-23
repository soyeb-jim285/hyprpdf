#pragma once
#include <QAbstractListModel>
#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStack>
#include <QTimer>
#include <QVariantList>
#include <QDateTime>

class AnnotationStore : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count          READ rowCount       NOTIFY countChanged)
    Q_PROPERTY(int revision       READ revision       NOTIFY revisionChanged)
    Q_PROPERTY(QString selectedId READ selectedId     WRITE setSelectedId NOTIFY selectedIdChanged)
    Q_PROPERTY(bool canUndo       READ canUndo        NOTIFY historyChanged)
    Q_PROPERTY(bool canRedo       READ canRedo        NOTIFY historyChanged)
    Q_PROPERTY(QString currentAuthor READ currentAuthor WRITE setCurrentAuthor NOTIFY currentAuthorChanged)

public:
    enum Type { Highlight = 0, Underline, Strikeout, StickyNote, Ink };
    Q_ENUM(Type)

    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole, PageRole, RectsRole, AnchorRole,
        InkStrokesRole, InkStrokeWidthRole,
        ColorRole, NoteRole, AuthorRole, CreatedAtRole, SelectedRole
    };

    struct Annot {
        QString id;
        int type = Highlight;
        int page = 0;
        QVector<QRectF> rects;
        QPointF anchor;
        QVector<QVector<QPointF>> strokes;
        qreal strokeWidth = 2.0;
        QColor color;
        QString note;
        QString author;
        QDateTime createdAt;
    };

    explicit AnnotationStore(QObject *parent = nullptr);
    ~AnnotationStore() override;

    Q_INVOKABLE void setStorageDir(const QString &dir);

    Q_INVOKABLE void loadDocument(const QString &hash);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void save();

    Q_INVOKABLE QString addHighlight(int page, QVariantList rects, const QColor &color);
    Q_INVOKABLE QString addUnderline(int page, QVariantList rects, const QColor &color);
    Q_INVOKABLE QString addStrikeout(int page, QVariantList rects, const QColor &color);
    Q_INVOKABLE QString addStickyNote(int page, QPointF anchor, const QColor &color,
                                      const QString &note);
    Q_INVOKABLE QString addInk(int page, QVariantList strokes, const QColor &color,
                               qreal strokeWidth);

    Q_INVOKABLE void remove(const QString &id);
    Q_INVOKABLE void setColor(const QString &id, const QColor &color);
    Q_INVOKABLE void setNote(const QString &id, const QString &note);

    Q_INVOKABLE QVariantList annotationsOnPage(int page) const;

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    Q_INVOKABLE void exportMarkdown(const QString &path) const;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int revision() const { return m_revision; }
    QString selectedId() const { return m_selectedId; }
    void setSelectedId(const QString &id);
    bool canUndo() const { return !m_undo.isEmpty(); }
    bool canRedo() const { return !m_redo.isEmpty(); }
    QString currentAuthor() const { return m_author; }
    void setCurrentAuthor(const QString &a);

signals:
    void countChanged();
    void revisionChanged();
    void selectedIdChanged();
    void historyChanged();
    void currentAuthorChanged();
    void saveFailed(QString reason);

private:
    enum CmdKind { CmdAdd, CmdRemove, CmdSetColor, CmdSetNote };
    struct Command {
        CmdKind kind;
        Annot before;
        Annot after;
    };

    QString addAnnotInternal(Annot a);
    int findIndex(const QString &id) const;
    QString sidecarPath() const;
    void scheduleSave();
    void pushUndo(Command c);
    void applyCommandForward(const Command &c);
    void applyCommandBackward(const Command &c);
    void bumpRevision();

    static constexpr int kUndoDepth = 50;

    QVector<Annot> m_annots;
    QString m_docHash;
    QString m_storageDir;
    QString m_author = "HyprPDF User";
    QString m_selectedId;
    QStack<Command> m_undo;
    QStack<Command> m_redo;
    QTimer m_saveTimer;
    int m_revision = 0;
    bool m_suppressHistory = false;
};
