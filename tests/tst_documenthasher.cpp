#include <QTest>
#include <QFile>
#include <QTemporaryFile>
#include "services/documenthasher.h"

class TestDocumentHasher : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture())) QSKIP("sample.pdf missing");
    }

    void hashSyncReturnsSha256HexString() {
        DocumentHasher h;
        const QString hash = h.hashFileSync(fixture());
        QCOMPARE(hash.length(), 64);
        for (QChar c : hash) {
            QVERIFY((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
        }
    }

    void hashIsStable() {
        DocumentHasher h;
        const QString a = h.hashFileSync(fixture());
        const QString b = h.hashFileSync(fixture());
        QCOMPARE(a, b);
    }

    void cacheHitAvoidsRehash() {
        DocumentHasher h;
        const QString a = h.hashFileSync(fixture());
        QVERIFY(h.isCached(fixture()));
        const QString b = h.hashFileSync(fixture());
        QCOMPARE(a, b);
    }

    void missingFileReturnsEmpty() {
        DocumentHasher h;
        QCOMPARE(h.hashFileSync("/no/such/file.pdf"), QString());
    }

    void differentFilesDifferentHashes() {
        DocumentHasher h;
        const QString a = h.hashFileSync(fixture());
        const QString m = h.hashFileSync(QString(HYPRPDF_FIXTURES_DIR) + "/multi.pdf");
        QVERIFY(!a.isEmpty());
        QVERIFY(!m.isEmpty());
        QVERIFY(a != m);
    }
};
QTEST_MAIN(TestDocumentHasher)
#include "tst_documenthasher.moc"
