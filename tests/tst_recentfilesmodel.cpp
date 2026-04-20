#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "models/recentfilesmodel.h"

class TestRecentFilesModel : public QObject {
    Q_OBJECT
private slots:
    void addRecentDedupesAndMovesToTop() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/a/b/foo.pdf");
        m.addRecent("/a/b/bar.pdf");
        m.addRecent("/a/b/foo.pdf");
        QCOMPARE(m.rowCount(), 2);
        QCOMPARE(m.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/a/b/foo.pdf"));
        QCOMPARE(m.index(1).data(RecentFilesModel::PathRole).toString(),
                 QString("/a/b/bar.pdf"));
    }

    void capsAt20() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        for (int i = 0; i < 25; ++i)
            m.addRecent(QString("/x/file%1.pdf").arg(i));
        QCOMPARE(m.rowCount(), 20);
        QCOMPARE(m.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/x/file24.pdf"));
    }

    void clearEmpties() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/a"); m.addRecent("/b");
        m.clear();
        QCOMPARE(m.rowCount(), 0);
    }

    void persistRoundtrip() {
        QTemporaryDir dir;
        const QString p = dir.filePath("recent.json");
        {
            RecentFilesModel m(p);
            m.addRecent("/x/one.pdf");
            m.addRecent("/x/two.pdf");
        }
        RecentFilesModel m2(p);
        QCOMPARE(m2.rowCount(), 2);
        QCOMPARE(m2.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/x/two.pdf"));
    }

    void existsRoleFalseForMissingPath() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/definitely/does/not/exist.pdf");
        QCOMPARE(m.index(0).data(RecentFilesModel::ExistsRole).toBool(), false);
    }

    void openEmitsRequest() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/p/q.pdf");
        QSignalSpy spy(&m, &RecentFilesModel::requestOpen);
        m.open(0);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().first().toString(), QString("/p/q.pdf"));
    }
};
QTEST_MAIN(TestRecentFilesModel)
#include "tst_recentfilesmodel.moc"
