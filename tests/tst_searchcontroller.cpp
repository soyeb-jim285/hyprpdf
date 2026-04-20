#include <QTest>
#include <QSignalSpy>
#include <QFile>
#include "services/searchcontroller.h"
#include "services/pdfdoc.h"

class TestSearchController : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture())) QSKIP("sample.pdf missing");
    }

    void setDocAndSearchFindsMatches() {
        auto d = PdfDoc::load(fixture());
        QVERIFY(d != nullptr);
        SearchController c;
        c.setDocument(d.get());
        c.search("the");
        QVERIFY(c.matchCount() > 0);
        QCOMPARE(c.currentIndex(), 0);
    }

    void nextWrapsAround() {
        auto d = PdfDoc::load(fixture());
        SearchController c;
        c.setDocument(d.get());
        c.search("the");
        const int n = c.matchCount();
        if (n < 2) QSKIP("need at least 2 matches");
        c.next();
        QCOMPARE(c.currentIndex(), 1);
        for (int i = 1; i < n; ++i) c.next();
        QCOMPARE(c.currentIndex(), 0);
    }

    void prevFromZeroWraps() {
        auto d = PdfDoc::load(fixture());
        SearchController c;
        c.setDocument(d.get());
        c.search("the");
        const int n = c.matchCount();
        if (n < 2) QSKIP("need at least 2 matches");
        c.prev();
        QCOMPARE(c.currentIndex(), n - 1);
    }

    void clearEmptiesResults() {
        auto d = PdfDoc::load(fixture());
        SearchController c;
        c.setDocument(d.get());
        c.search("the");
        c.clear();
        QCOMPARE(c.matchCount(), 0);
        QCOMPARE(c.currentIndex(), -1);
    }

    void setNullDocumentClears() {
        auto d = PdfDoc::load(fixture());
        SearchController c;
        c.setDocument(d.get());
        c.search("the");
        c.setDocument(nullptr);
        QCOMPARE(c.matchCount(), 0);
    }
};
QTEST_MAIN(TestSearchController)
#include "tst_searchcontroller.moc"
