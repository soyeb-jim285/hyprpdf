#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "services/themeloader.h"

class TestThemeLoader : public QObject {
    Q_OBJECT
private slots:
    void defaultsWithoutTomlFile() {
        ThemeLoader t;
        QCOMPARE(t.color("base"),    QColor("#1e1e2e"));
        QCOMPARE(t.color("text"),    QColor("#cdd6f4"));
        QCOMPARE(t.color("accent"),  QColor("#89b4fa"));
        QCOMPARE(t.annotation("highlight"), QColor("#f9e2af"));
        QCOMPARE(t.annotation("ink"),       QColor("#89b4fa"));
    }

    void loadOverridesFromToml() {
        QTemporaryDir dir;
        QFile f(dir.filePath("custom.toml"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("name = \"custom\"\n"
                "[colors]\n"
                "base   = \"#000000\"\n"
                "text   = \"#ffffff\"\n"
                "[annotations]\n"
                "highlight = \"#ff0000\"\n");
        f.close();
        ThemeLoader t;
        t.loadTheme("custom", dir.path());
        QCOMPARE(t.color("base"),  QColor("#000000"));
        QCOMPARE(t.color("text"),  QColor("#ffffff"));
        QCOMPARE(t.annotation("highlight"), QColor("#ff0000"));
        QCOMPARE(t.color("accent"), QColor("#89b4fa"));
    }

    void themeChangedSignalFires() {
        QTemporaryDir dir;
        QFile f(dir.filePath("x.toml"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("name = \"x\"\n[colors]\nbase=\"#abcdef\"\n");
        f.close();
        ThemeLoader t;
        QSignalSpy spy(&t, &ThemeLoader::themeChanged);
        t.loadTheme("x", dir.path());
        QCOMPARE(spy.count(), 1);
    }

    void availableThemesListsTomlFiles() {
        QTemporaryDir dir;
        for (auto n : {"a.toml", "b.toml", "ignore.txt"}) {
            QFile f(dir.filePath(n)); QVERIFY(f.open(QIODevice::WriteOnly)); f.close();
        }
        ThemeLoader t;
        auto list = t.availableThemes(dir.path());
        QCOMPARE(list.size(), 2);
        QVERIFY(list.contains("a"));
        QVERIFY(list.contains("b"));
    }
};
QTEST_MAIN(TestThemeLoader)
#include "tst_themeloader.moc"
