#include "documentcontroller.h"
#include "models/documentmodel.h"
#include <QFileInfo>

DocumentController::DocumentController(DocumentModel *model, QObject *parent)
    : QObject(parent), m_model(model) {
    connect(m_model, &DocumentModel::openFailed, this,
            [this](const QString &p, const QString &r) {
                emit openFailed(p + ": " + r);
            });
}

void DocumentController::open(const QString &path) {
    if (path.isEmpty()) { emit openFailed("empty path"); return; }
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "pdf") {
        m_model->openDocument(path);
        return;
    }
    static const QStringList kOffice = {
        "pptx","ppt","odp","docx","doc","odt","rtf","xlsx","ods","csv",
        "epub","md","html"
    };
    if (kOffice.contains(ext)) {
        emit openFailed(QString("%1: office formats land in v0.5").arg(path));
        return;
    }
    emit openFailed(QString("%1: unsupported file type .%2").arg(path, ext));
}

void DocumentController::closeCurrent() {
    if (m_model->currentIndex() >= 0)
        m_model->closeTab(m_model->currentIndex());
}

void DocumentController::openFromUrl(const QUrl &url) {
    open(url.toLocalFile());
}
