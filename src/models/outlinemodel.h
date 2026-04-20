#pragma once
#include <QAbstractItemModel>
#include <QString>
#include <QVector>

class PdfDoc;

class OutlineModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PageRole,
        DepthRole
    };

    explicit OutlineModel(QObject *parent = nullptr);
    ~OutlineModel() override;

    Q_INVOKABLE void setDocument(QObject *doc);

    QModelIndex index(int row, int col, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    struct Node {
        QString title;
        int page = -1;
        int depth = 0;
        Node *parent = nullptr;
        QVector<Node*> children;
    };

private:
    void clearRoot();
    Node *m_root;
};
