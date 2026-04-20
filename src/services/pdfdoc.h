#pragma once
#include <QObject>
#include <QHash>
#include <QString>
#include <QSizeF>
#include <QImage>
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

private:
    explicit PdfDoc(std::unique_ptr<Poppler::Document> doc, QObject *parent = nullptr);

    int m_id;
    std::unique_ptr<Poppler::Document> m_doc;

    static int s_nextId;
    static QHash<int, PdfDoc*> s_registry;
};
