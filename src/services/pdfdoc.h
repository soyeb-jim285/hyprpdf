#pragma once
#include <QObject>
#include <QHash>
#include <QString>
#include <QSizeF>
#include <QImage>
#include <QRectF>
#include <QVector>
#include <QVariant>
#include <memory>

namespace Poppler { class Document; }

class PdfDoc : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id           READ id           CONSTANT)
    Q_PROPERTY(int pageCount    READ pageCount    CONSTANT)
    Q_PROPERTY(QString title    READ title        CONSTANT)
public:
    // Factory. Returns nullptr on load failure.
    static std::unique_ptr<PdfDoc> load(const QString &path, QObject *parent = nullptr);

    // Global registry (id -> PdfDoc*). Populated in ctor, cleared in dtor.
    static PdfDoc *byId(int id);

    ~PdfDoc() override;

    int id() const { return m_id; }
    bool isValid() const;
    int pageCount() const;
    QString title() const;
    Q_INVOKABLE QSizeF pageSize(int index) const;
    Q_INVOKABLE QString text(int index) const;

    // Renders page at given target width (px). Height follows page aspect.
    QImage renderPage(int index, int targetWidth) const;

    struct TocEntry {
        QString title;
        int pageIndex;
        QVector<TocEntry> children;
    };

    Q_INVOKABLE QVariantList search(int page, const QString &text) const;
    Q_INVOKABLE QString textInRect(int page, const QRectF &rect) const;
    // Word-level bounding rects (PDF points) that intersect the rect.
    Q_INVOKABLE QVariantList selectionRects(int page, const QRectF &rect) const;
    // Text-editor-style line-aware selection from start point to end point
    // (both in PDF points). Returns word bboxes from start through end in
    // reading order; start line: words to the right of startX; middle lines:
    // every word; end line: words to the left of endX.
    Q_INVOKABLE QVariantList selectionRectsLine(int page,
                                                qreal sx, qreal sy,
                                                qreal ex, qreal ey) const;
    const QVector<TocEntry> &toc() const;

private:
    explicit PdfDoc(std::unique_ptr<Poppler::Document> doc, QObject *parent = nullptr);

    int m_id;
    std::unique_ptr<Poppler::Document> m_doc;
    mutable QVector<TocEntry> m_toc;
    mutable bool m_tocBuilt = false;

    static int s_nextId;
    static QHash<int, PdfDoc*> s_registry;
};
