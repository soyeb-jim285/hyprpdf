#pragma once
#include <QObject>
#include <QString>
#include <QRectF>
#include <QList>
#include <QVariant>
#include <QVariantList>

class PdfDoc;

class SearchController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString query       READ query        NOTIFY queryChanged)
    Q_PROPERTY(int matchCount      READ matchCount   NOTIFY resultsChanged)
    Q_PROPERTY(int currentIndex    READ currentIndex NOTIFY currentChanged)
    Q_PROPERTY(int currentPage     READ currentPage  NOTIFY currentChanged)
    Q_PROPERTY(QRectF currentRect  READ currentRect  NOTIFY currentChanged)
    // Monotonic version counter, incremented on every results mutation.
    // QML bindings read this to set up a reactive dep on resultsChanged.
    Q_PROPERTY(int revision        READ revision     NOTIFY resultsChanged)

public:
    struct Match { int page; QRectF rect; };

    explicit SearchController(QObject *parent = nullptr);

    QString query() const { return m_query; }
    int matchCount() const { return m_matches.size(); }
    int currentIndex() const { return m_current; }
    int currentPage() const;
    QRectF currentRect() const;
    int revision() const { return m_revision; }

    Q_INVOKABLE void setDocument(QObject *doc);
    Q_INVOKABLE void search(const QString &q);
    Q_INVOKABLE void next();
    Q_INVOKABLE void prev();
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantList matchesOnPage(int page) const;

signals:
    void queryChanged();
    void resultsChanged();
    void currentChanged();
    void scrollToRequested(int page, QRectF rect);

private:
    void emitCurrent();

    PdfDoc *m_doc = nullptr;
    QString m_query;
    QList<Match> m_matches;
    int m_current = -1;
    int m_revision = 0;
};
