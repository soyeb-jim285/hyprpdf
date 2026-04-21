#include "clipboardbridge.h"
#include <QGuiApplication>
#include <QClipboard>

ClipboardBridge::ClipboardBridge(QObject *parent) : QObject(parent) {}

void ClipboardBridge::setText(const QString &s) {
    if (auto *cb = QGuiApplication::clipboard()) cb->setText(s);
}

QString ClipboardBridge::text() const {
    if (auto *cb = QGuiApplication::clipboard()) return cb->text();
    return {};
}
