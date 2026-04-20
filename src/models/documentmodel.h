#pragma once
#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>
#include <memory>

class PdfDoc;

class DocumentModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count        READ rowCount     NOTIFY countChanged)
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        TitleRole,
        PageCountRole,
        IsDirtyRole,
        DocumentRole
    };

    explicit DocumentModel(QObject *parent = nullptr);
    ~DocumentModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int  currentIndex() const { return m_current; }
    void setCurrentIndex(int idx);

    Q_INVOKABLE int  openDocument(const QString &path);
    Q_INVOKABLE void closeTab(int index);

signals:
    void currentIndexChanged();
    void countChanged();
    void openFailed(QString path, QString reason);

private:
    struct Tab {
        QString path;
        QString title;
        std::unique_ptr<PdfDoc> doc;
        bool dirty = false;
    };
    QVector<Tab*> m_tabs;
    int m_current = -1;
};
