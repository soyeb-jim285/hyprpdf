#include "themeloader.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QDebug>

#define TOML_HEADER_ONLY 1
#include "third_party/toml.hpp"

namespace {

const QHash<QString, QColor> kDefaultColors = {
    {"base",    QColor("#1e1e2e")},
    {"mantle",  QColor("#181825")},
    {"crust",   QColor("#11111b")},
    {"surface", QColor("#313244")},
    {"overlay", QColor("#6c7086")},
    {"text",    QColor("#cdd6f4")},
    {"subtext", QColor("#bac2de")},
    {"muted",   QColor("#a6adc8")},
    {"accent",  QColor("#89b4fa")},
    {"success", QColor("#a6e3a1")},
    {"warning", QColor("#f9e2af")},
    {"error",   QColor("#f38ba8")},
};

const QHash<QString, QColor> kDefaultAnnotations = {
    {"highlight", QColor("#f9e2af")},
    {"underline", QColor("#a6e3a1")},
    {"strikeout", QColor("#f38ba8")},
    {"ink",       QColor("#89b4fa")},
    {"note",      QColor("#fab387")},
};

QString resolveThemePath(const QString &nameOrPath, const QString &themesDir) {
    if (nameOrPath.isEmpty()) return {};
    QFileInfo fi(nameOrPath);
    if (fi.isAbsolute() && fi.exists()) return nameOrPath;
    QString candidate = QDir(themesDir).filePath(nameOrPath.endsWith(".toml")
                          ? nameOrPath : nameOrPath + ".toml");
    return QFile::exists(candidate) ? candidate : QString{};
}

}  // namespace

ThemeLoader::ThemeLoader(QObject *parent) : QObject(parent) {}

QColor ThemeLoader::color(const QString &name) const {
    if (auto it = m_colors.constFind(name); it != m_colors.cend()) return *it;
    return kDefaultColors.value(name);
}

QColor ThemeLoader::annotation(const QString &name) const {
    if (auto it = m_annotations.constFind(name); it != m_annotations.cend()) return *it;
    return kDefaultAnnotations.value(name);
}

void ThemeLoader::loadTheme(const QString &nameOrPath, const QString &themesDir) {
    const QString path = resolveThemePath(nameOrPath, themesDir);
    if (path.isEmpty()) {
        qWarning() << "ThemeLoader: theme not found" << nameOrPath << "in" << themesDir;
        emit themeChanged();
        return;
    }
    try {
        auto tbl = toml::parse_file(path.toStdString());
        m_colors.clear();
        m_annotations.clear();
        if (auto colors = tbl["colors"].as_table()) {
            for (auto &[k, v] : *colors) {
                if (auto s = v.value<std::string>()) {
                    m_colors.insert(QString::fromStdString(std::string(k.str())),
                                    QColor(QString::fromStdString(*s)));
                }
            }
        }
        if (auto annots = tbl["annotations"].as_table()) {
            for (auto &[k, v] : *annots) {
                if (auto s = v.value<std::string>()) {
                    m_annotations.insert(QString::fromStdString(std::string(k.str())),
                                         QColor(QString::fromStdString(*s)));
                }
            }
        }
    } catch (const toml::parse_error &e) {
        qWarning() << "ThemeLoader: parse error" << e.what();
    }
    emit themeChanged();
}

QStringList ThemeLoader::availableThemes(const QString &themesDir) const {
    QStringList out;
    QDir dir(themesDir);
    for (const auto &fi : dir.entryInfoList({"*.toml"}, QDir::Files)) {
        out << fi.completeBaseName();
    }
    return out;
}
