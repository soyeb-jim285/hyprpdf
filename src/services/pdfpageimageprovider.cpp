#include "pdfpageimageprovider.h"
#include "pdfdoc.h"
#include <QStringList>
#include <QDebug>

PdfPageImageProvider::PdfPageImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {}

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
    PdfDoc *doc = PdfDoc::byId(docId);
    if (!doc) {
        qWarning() << "PdfPageImageProvider: unknown docId" << docId;
        return {};
    }
    QImage img = doc->renderPage(pageIndex, targetWidth);
    if (size) *size = img.size();
    return img;
}
