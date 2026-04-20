#include "configmanager.h"
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#define TOML_HEADER_ONLY 1
#include "third_party/toml.hpp"

ConfigManager::ConfigManager(const QString &path, QObject *parent)
    : QObject(parent) {
    if (path.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                          + "/hyprpdf";
        QDir().mkpath(dir);
        m_path = dir + "/config.toml";
    } else {
        m_path = path;
        QDir().mkpath(QFileInfo(path).absolutePath());
    }
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(300);
    connect(&m_saveTimer, &QTimer::timeout, this, &ConfigManager::save);

    load();
}

void ConfigManager::load() {
    if (!QFile::exists(m_path)) {
        save();
        return;
    }
    try {
        auto tbl = toml::parse_file(m_path.toStdString());
        if (auto v = tbl["theme"].value<std::string>())             m_theme = QString::fromStdString(*v);
        if (auto v = tbl["iconTheme"].value<std::string>())         m_iconTheme = QString::fromStdString(*v);
        if (auto v = tbl["defaultZoom"].value<double>())            m_defaultZoom = *v;
        if (auto v = tbl["continuousScroll"].value<bool>())         m_continuousScroll = *v;
        if (auto v = tbl["radiusSm"].value<int64_t>())              m_radiusSm = static_cast<int>(*v);
        if (auto v = tbl["radiusMd"].value<int64_t>())              m_radiusMd = static_cast<int>(*v);
        if (auto v = tbl["radiusLg"].value<int64_t>())              m_radiusLg = static_cast<int>(*v);
        if (auto v = tbl["animationsEnabled"].value<bool>())        m_animationsEnabled = *v;
        if (auto v = tbl["animDurationFast"].value<int64_t>())      m_animDurationFast = static_cast<int>(*v);
        if (auto v = tbl["animDurationNormal"].value<int64_t>())    m_animDurationNormal = static_cast<int>(*v);
        if (auto v = tbl["animDurationSlow"].value<int64_t>())      m_animDurationSlow = static_cast<int>(*v);
        if (auto v = tbl["transparencyEnabled"].value<bool>())      m_transparencyEnabled = *v;
        if (auto v = tbl["transparencyLevel"].value<double>())      m_transparencyLevel = *v;
    } catch (const toml::parse_error &e) {
        qWarning() << "ConfigManager: parse error" << e.what() << "rotating";
        rotateBroken();
        save();
    }
}

void ConfigManager::rotateBroken() {
    const QString rotated = m_path + ".broken-"
        + QString::number(QDateTime::currentSecsSinceEpoch());
    QFile::rename(m_path, rotated);
}

void ConfigManager::save() {
    QSaveFile f(m_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "ConfigManager: cannot write" << m_path << f.errorString();
        return;
    }
    QString s;
    s += "theme               = \"" + m_theme + "\"\n";
    s += "iconTheme           = \"" + m_iconTheme + "\"\n";
    s += "defaultZoom         = " + QString::number(m_defaultZoom) + "\n";
    s += QString("continuousScroll    = %1\n").arg(m_continuousScroll ? "true" : "false");
    s += "radiusSm            = " + QString::number(m_radiusSm) + "\n";
    s += "radiusMd            = " + QString::number(m_radiusMd) + "\n";
    s += "radiusLg            = " + QString::number(m_radiusLg) + "\n";
    s += QString("animationsEnabled   = %1\n").arg(m_animationsEnabled ? "true" : "false");
    s += "animDurationFast    = " + QString::number(m_animDurationFast) + "\n";
    s += "animDurationNormal  = " + QString::number(m_animDurationNormal) + "\n";
    s += "animDurationSlow    = " + QString::number(m_animDurationSlow) + "\n";
    s += QString("transparencyEnabled = %1\n").arg(m_transparencyEnabled ? "true" : "false");
    s += "transparencyLevel   = " + QString::number(m_transparencyLevel) + "\n";
    f.write(s.toUtf8());
    f.commit();
}

void ConfigManager::reload() {
    load();
    emit configChanged();
}

void ConfigManager::scheduleSave() {
    emit configChanged();
    m_saveTimer.start();
}

#define SETTER(Type, Name, Member) \
    void ConfigManager::set##Name(Type v) { if (Member == v) return; Member = v; scheduleSave(); }

SETTER(const QString &, Theme,            m_theme)
SETTER(const QString &, IconTheme,        m_iconTheme)
SETTER(double,          DefaultZoom,      m_defaultZoom)
SETTER(bool,            ContinuousScroll, m_continuousScroll)
SETTER(int,             RadiusSm,         m_radiusSm)
SETTER(int,             RadiusMd,         m_radiusMd)
SETTER(int,             RadiusLg,         m_radiusLg)
SETTER(bool,            AnimationsEnabled,   m_animationsEnabled)
SETTER(int,             AnimDurationFast,    m_animDurationFast)
SETTER(int,             AnimDurationNormal,  m_animDurationNormal)
SETTER(int,             AnimDurationSlow,    m_animDurationSlow)
SETTER(bool,            TransparencyEnabled, m_transparencyEnabled)
SETTER(double,          TransparencyLevel,   m_transparencyLevel)
