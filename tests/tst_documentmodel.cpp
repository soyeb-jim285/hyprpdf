#include <QTest>
#include <QSignalSpy>
#include <QFile>
#include "models/documentmodel.h"

class TestDocumentModel : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture())) QSKIP("sample.pdf fixture missing");
    }

    void openAddsRowAndDedupes() {
        DocumentModel m;
        const int idx1 = m.openDocument(fixture());
        QCOMPARE(idx1, 0);
        QCOMPARE(m.rowCount(), 1);
        const int idx2 = m.openDocument(fixture());     // dedupe
        QCOMPARE(idx2, 0);
        QCOMPARE(m.rowCount(), 1);
    }

    void closeTabRemoves() {
        DocumentModel m;
        m.openDocument(fixture());
        QCOMPARE(m.rowCount(), 1);
        m.closeTab(0);
        QCOMPARE(m.rowCount(), 0);
    }

    void currentIndexClamps() {
        DocumentModel m;
        m.openDocument(fixture());
        m.setCurrentIndex(99);
        QCOMPARE(m.currentIndex(), 0);
        m.setCurrentIndex(-5);
        QCOMPARE(m.currentIndex(), 0);
    }

    void openFailedEmitsOnBogusPath() {
        DocumentModel m;
        QSignalSpy spy(&m, &DocumentModel::openFailed);
        const int idx = m.openDocument("/no/such/file.pdf");
        QCOMPARE(idx, -1);
        QCOMPARE(spy.count(), 1);
    }

    void documentRoleReturnsPdfDoc() {
        DocumentModel m;
        m.openDocument(fixture());
        auto var = m.index(0).data(DocumentModel::DocumentRole);
        QObject *obj = var.value<QObject*>();
        QVERIFY(obj != nullptr);
        // PdfDoc exposes pageCount; verify via dynamic call
        bool ok = QMetaObject::invokeMethod(obj, "pageCount",
                    Qt::DirectConnection, Q_RETURN_ARG(int, *new int{0}));
        Q_UNUSED(ok);
        // Simpler: dynamic property read
        const int pageCount = obj->property("pageCount").toInt();
        QVERIFY(pageCount >= 1);
    }
};
QTEST_MAIN(TestDocumentModel)
#include "tst_documentmodel.moc"
