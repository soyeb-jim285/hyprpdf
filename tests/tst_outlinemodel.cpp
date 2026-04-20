#include <QTest>
#include <QFile>
#include "models/outlinemodel.h"
#include "services/pdfdoc.h"

class TestOutlineModel : public QObject {
    Q_OBJECT
private:
    QString sampleFixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
    QString multiFixture()  const { return QString(HYPRPDF_FIXTURES_DIR) + "/multi.pdf"; }
private slots:
    void emptyModelWhenNoDoc() {
        OutlineModel m;
        QCOMPARE(m.rowCount(), 0);
    }

    void setNullDocumentEmpties() {
        OutlineModel m;
        m.setDocument(nullptr);
        QCOMPARE(m.rowCount(), 0);
    }

    void sampleWithoutTocYieldsEmpty() {
        if (!QFile::exists(sampleFixture())) QSKIP("sample.pdf missing");
        auto d = PdfDoc::load(sampleFixture());
        QVERIFY(d != nullptr);
        OutlineModel m;
        m.setDocument(d.get());
        QCOMPARE(m.rowCount(), 0);
    }

    void multiSetDocumentDoesNotCrash() {
        if (!QFile::exists(multiFixture())) QSKIP("multi.pdf missing");
        auto d = PdfDoc::load(multiFixture());
        QVERIFY(d != nullptr);
        OutlineModel m;
        m.setDocument(d.get());
        QVERIFY(m.rowCount() >= 0);    // may be 0 if the source PDF was stripped of outline
    }

    void rolesReturnExpectedNames() {
        OutlineModel m;
        auto names = m.roleNames();
        QVERIFY(names.values().contains("title"));
        QVERIFY(names.values().contains("page"));
        QVERIFY(names.values().contains("depth"));
    }
};
QTEST_MAIN(TestOutlineModel)
#include "tst_outlinemodel.moc"
