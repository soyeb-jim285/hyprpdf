#pragma once
#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QString>
#include <QVector>

class RecentFilesModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        NameRole,
        SizeRole,
        ModifiedRole,
        IconNameRole,
        ExistsRole
    };

    explicit RecentFilesModel(const QString &path = {}, QObject *parent = nullptr);
    ~RecentFilesModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addRecent(const QString &path);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void open(int index);

signals:
    void requestOpen(const QString &path);

private:
    struct Entry { QString path; QDateTime addedAt; };
    void load();
    void save();
    static QString iconNameFor(const QString &path);

    QString m_storePath;
    QVector<Entry> m_entries;
    static constexpr int kMax = 20;
};
