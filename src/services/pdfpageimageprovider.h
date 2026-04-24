#pragma once
#include <QQuickImageProvider>
#include <QCache>
#include <QImage>
#include <QString>

class PdfPageImageProvider : public QQuickImageProvider {
public:
    PdfPageImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    // Keyed by "<docId>/<pageIndex>/<targetWidth>". Capped at ~100 MB.
    // QCache takes ownership of inserted QImage pointers.
    QCache<QString, QImage> m_cache;
};
