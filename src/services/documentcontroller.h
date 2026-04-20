#pragma once
#include <QObject>
#include <QString>
#include <QUrl>

class DocumentModel;

class DocumentController : public QObject {
    Q_OBJECT
public:
    explicit DocumentController(DocumentModel *model, QObject *parent = nullptr);

    Q_INVOKABLE void open(const QString &path);
    Q_INVOKABLE void closeCurrent();
    Q_INVOKABLE void openFromUrl(const QUrl &url);

signals:
    void openFailed(QString reason);

private:
    DocumentModel *m_model;
};
