#pragma once
#include <QQuickImageProvider>

class PdfPageImageProvider : public QQuickImageProvider {
public:
    PdfPageImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};
