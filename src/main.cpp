#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("HyprPDF");
    QGuiApplication::setApplicationVersion(HYPRPDF_VERSION);
    QQuickStyle::setStyle("Basic");

    QCommandLineParser parser;
    parser.setApplicationDescription("HyprPDF — Qt6/QML PDF viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "PDF or office document to open", "[file]");
    parser.process(app);

    QString initialFile;
    const auto positional = parser.positionalArguments();
    if (!positional.isEmpty())
        initialFile = positional.first();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"initialFile", initialFile}});
    engine.loadFromModule("HyprPDF", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
