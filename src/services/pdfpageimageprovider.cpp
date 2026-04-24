#include "pdfpageimageprovider.h"
#include "pdfdoc.h"
#include <QStringList>
#include <QDebug>

// ~100 MB image cache cap (QCache cost is in bytes here)
static constexpr int kCacheMaxBytes = 100 * 1024 * 1024;

PdfPageImageProvider::PdfPageImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_cache(kCacheMaxBytes)
{}

QImage PdfPageImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    // id format: "<docId>/<pageIndex>/<targetWidth>"
    const auto parts = id.split('/');
    if (parts.size() != 3) {
        qWarning() << "PdfPageImageProvider: bad id" << id;
        return {};
    }
    bool ok1 = false, ok2 = false, ok3 = false;
    const int docId      = parts[0].toInt(&ok1);
    const int pageIndex  = parts[1].toInt(&ok2);
    int targetWidth      = parts[2].toInt(&ok3);
    if (!ok1 || !ok2 || !ok3) {
        qWarning() << "PdfPageImageProvider: parse fail" << id;
        return {};
    }
    if (requestedSize.width() > 0) targetWidth = requestedSize.width();

    // Build cache key including the effective targetWidth.
    const QString cacheKey = parts[0] + '/' + parts[1] + '/' + QString::number(targetWidth);

    if (QImage *cached = m_cache.object(cacheKey)) {
        if (size) *size = cached->size();
        return *cached;
    }

    PdfDoc *doc = PdfDoc::byId(docId);
    if (!doc) {
        qWarning() << "PdfPageImageProvider: unknown docId" << docId;
        return {};
    }
    QImage img = doc->renderPage(pageIndex, targetWidth);
    if (size) *size = img.size();
    if (!img.isNull()) {
        const int cost = img.sizeInBytes();
        m_cache.insert(cacheKey, new QImage(img), cost);
    }
    return img;
}
