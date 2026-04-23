#pragma once
#include <QObject>
#include <QString>
#include <QCache>
#include <QMutex>
#include <QFuture>

class DocumentHasher : public QObject {
    Q_OBJECT
public:
    explicit DocumentHasher(QObject *parent = nullptr);

    // Async — resolves to lowercase hex SHA-256 or empty string on failure.
    QFuture<QString> hashFile(const QString &path);
    // Sync fallback — blocks on this thread.
    Q_INVOKABLE QString hashFileSync(const QString &path);
    // Test helper — cache hit check keyed by path + mtime + size.
    bool isCached(const QString &path) const;

private:
    static QString computeSha256(const QString &path);
    static QString cacheKey(const QString &path);

    mutable QMutex m_mutex;
    QCache<QString, QString> m_cache;
};
