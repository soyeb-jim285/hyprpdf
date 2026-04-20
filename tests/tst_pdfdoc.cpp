#include <QTest>
#include <QFile>
#include "services/pdfdoc.h"

class TestPdfDoc : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture())) QSKIP("sample.pdf fixture missing");
    }

    void loadValidPdf() {
        auto d = PdfDoc::load(fixture());
        QVERIFY(d != nullptr);
        QVERIFY(d->isValid());
        QVERIFY(d->pageCount() >= 1);
    }

    void loadBogusPath() {
        auto d = PdfDoc::load("/no/such/file.pdf");
        QVERIFY(d == nullptr);
    }

    void pageSizeReasonable() {
        auto d = PdfDoc::load(fixture());
        QVERIFY(d != nullptr);
        QSizeF s = d->pageSize(0);
        QVERIFY(s.width()  > 100);
        QVERIFY(s.height() > 100);
    }

    void idIsUnique() {
        auto a = PdfDoc::load(fixture());
        auto b = PdfDoc::load(fixture());
        QVERIFY(a && b);
        QVERIFY(a->id() != b->id());
    }

    void registryLookupRoundtrip() {
        auto d = PdfDoc::load(fixture());
        QVERIFY(d != nullptr);
        int id = d->id();
        QCOMPARE(PdfDoc::byId(id), d.get());
        d.reset();
        QCOMPARE(PdfDoc::byId(id), nullptr);
    }
};
QTEST_MAIN(TestPdfDoc)
#include "tst_pdfdoc.moc"
