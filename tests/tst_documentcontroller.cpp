#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QFile>
#include "services/documentcontroller.h"
#include "models/documentmodel.h"

class TestDocumentController : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture())) QSKIP("sample.pdf fixture missing");
    }

    void openPdfRoutesToModel() {
        DocumentModel model;
        DocumentController c(&model);
        c.open(fixture());
        QCOMPARE(model.rowCount(), 1);
    }

    void openOfficeEmitsFailed() {
        DocumentModel model;
        DocumentController c(&model);
        QSignalSpy spy(&c, &DocumentController::openFailed);
        c.open("/tmp/example.docx");
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().first().toString().contains("v0.5"));
    }

    void openFromUrlStripsFilePrefix() {
        DocumentModel model;
        DocumentController c(&model);
        c.openFromUrl(QUrl::fromLocalFile(fixture()));
        QCOMPARE(model.rowCount(), 1);
    }

    void openUnknownExtensionEmitsFailed() {
        DocumentModel model;
        DocumentController c(&model);
        QSignalSpy spy(&c, &DocumentController::openFailed);
        c.open("/tmp/file.xyz");
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().first().toString().contains("unsupported"));
    }
};
QTEST_MAIN(TestDocumentController)
#include "tst_documentcontroller.moc"
