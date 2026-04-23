#include "documenthasher.h"
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QtConcurrent/QtConcurrent>

DocumentHasher::DocumentHasher(QObject *parent)
    : QObject(parent), m_cache(64) {}

QString DocumentHasher::cacheKey(const QString &path) {
    const QFileInfo fi(path);
    if (!fi.exists()) return {};
    return QString("%1#%2#%3")
        .arg(fi.absoluteFilePath())
        .arg(fi.lastModified().toMSecsSinceEpoch())
        .arg(fi.size());
}

QString DocumentHasher::computeSha256(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QCryptographicHash h(QCryptographicHash::Sha256);
    constexpr qint64 kChunk = 8 * 1024 * 1024;
    while (!f.atEnd()) {
        const QByteArray buf = f.read(kChunk);
        if (buf.isEmpty()) break;
        h.addData(buf);
    }
    return QString::fromLatin1(h.result().toHex());
}

QString DocumentHasher::hashFileSync(const QString &path) {
    const QString key = cacheKey(path);
    if (key.isEmpty()) return {};
    {
        QMutexLocker lock(&m_mutex);
        if (auto *v = m_cache.object(key)) return *v;
    }
    const QString digest = computeSha256(path);
    if (!digest.isEmpty()) {
        QMutexLocker lock(&m_mutex);
        m_cache.insert(key, new QString(digest));
    }
    return digest;
}

bool DocumentHasher::isCached(const QString &path) const {
    const QString key = cacheKey(path);
    if (key.isEmpty()) return false;
    QMutexLocker lock(&m_mutex);
    return m_cache.contains(key);
}

QFuture<QString> DocumentHasher::hashFile(const QString &path) {
    return QtConcurrent::run([this, path]() { return hashFileSync(path); });
}
