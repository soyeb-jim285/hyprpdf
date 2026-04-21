#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

#include "services/themeloader.h"
#include "services/configmanager.h"
#include "services/documentcontroller.h"
#include "services/pdfpageimageprovider.h"
#include "services/searchcontroller.h"
#include "services/clipboardbridge.h"
#include "models/documentmodel.h"
#include "models/recentfilesmodel.h"
#include "models/outlinemodel.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("HyprPDF");
    QGuiApplication::setApplicationVersion(HYPRPDF_VERSION);
    QGuiApplication::setOrganizationName("hyprpdf");
    QQuickStyle::setStyle("Basic");

    QCommandLineParser parser;
    parser.setApplicationDescription("HyprPDF - Qt6/QML PDF viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "PDF or office document to open", "[file]");
    parser.process(app);

    QString initialFile;
    const auto positional = parser.positionalArguments();
    if (!positional.isEmpty())
        initialFile = positional.first();

    auto *config        = new ConfigManager(QString{}, &app);
    auto *theme         = new ThemeLoader(&app);
    auto *recentFiles   = new RecentFilesModel(QString{}, &app);
    auto *documentModel = new DocumentModel(&app);
    auto *documentCtl   = new DocumentController(documentModel, &app);
    auto *searchController = new SearchController(&app);
    auto *outlineModel     = new OutlineModel(&app);
    auto *clipboard        = new ClipboardBridge(&app);

    const QString themesDir = QString(HYPRPDF_DATA_DIR) + "/themes";
    theme->loadTheme(config->theme(), themesDir);
    QObject::connect(config, &ConfigManager::configChanged, theme, [theme, config, themesDir]() {
        theme->loadTheme(config->theme(), themesDir);
    });

    QQmlApplicationEngine engine;
    engine.addImportPath(QString(HYPRPDF_DATA_DIR) + "/src/qml");
    engine.addImportPath(QString(HYPRPDF_DATA_DIR));
    engine.addImportPath(QString(HYPRPDF_SOURCE_DIR) + "/src/qml");
    engine.addImportPath(QString(HYPRPDF_SOURCE_DIR));

    // QQmlApplicationEngine takes ownership of the image provider
    engine.addImageProvider("pdf", new PdfPageImageProvider());

    auto *ctx = engine.rootContext();
    ctx->setContextProperty("config",             config);
    ctx->setContextProperty("theme",              theme);
    ctx->setContextProperty("recentFiles",        recentFiles);
    ctx->setContextProperty("documentModel",      documentModel);
    ctx->setContextProperty("documentController", documentCtl);
    ctx->setContextProperty("searchController", searchController);
    ctx->setContextProperty("outlineModel",     outlineModel);
    ctx->setContextProperty("clipboard",        clipboard);

    QObject::connect(recentFiles, &RecentFilesModel::requestOpen,
                     documentCtl, &DocumentController::open);
    QObject::connect(documentModel, &DocumentModel::countChanged,
                     recentFiles, [recentFiles, documentModel]() {
                         if (documentModel->rowCount() == 0) return;
                         const auto idx = documentModel->index(documentModel->rowCount() - 1, 0);
                         const auto path = idx.data(DocumentModel::PathRole).toString();
                         if (!path.isEmpty()) recentFiles->addRecent(path);
                     });

    engine.setInitialProperties({{"initialFile", initialFile}});
    engine.loadFromModule("HyprPDF", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
