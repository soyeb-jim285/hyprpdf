#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "services/configmanager.h"

class TestConfigManager : public QObject {
    Q_OBJECT
private slots:
    void firstRunWritesDefaults() {
        QTemporaryDir dir;
        ConfigManager c(dir.filePath("config.toml"));
        QVERIFY(QFile::exists(dir.filePath("config.toml")));
        QCOMPARE(c.theme(), QString("default"));
        QCOMPARE(c.iconTheme(), QString("default"));
        QCOMPARE(c.defaultZoom(), 1.0);
        QCOMPARE(c.continuousScroll(), true);
        QCOMPARE(c.radiusSm(), 4);
        QCOMPARE(c.radiusMd(), 8);
        QCOMPARE(c.radiusLg(), 12);
        QCOMPARE(c.animationsEnabled(), true);
        QCOMPARE(c.animDurationFast(), 100);
        QCOMPARE(c.animDurationNormal(), 200);
        QCOMPARE(c.animDurationSlow(), 350);
        QCOMPARE(c.transparencyEnabled(), false);
        QCOMPARE(c.transparencyLevel(), 0.85);
    }

    void setterSavesAndReloads() {
        QTemporaryDir dir;
        const QString path = dir.filePath("config.toml");
        {
            ConfigManager c(path);
            c.setTheme("latte");
            c.setDefaultZoom(1.5);
            c.save();
        }
        ConfigManager c2(path);
        QCOMPARE(c2.theme(), QString("latte"));
        QCOMPARE(c2.defaultZoom(), 1.5);
    }

    void configChangedSignalFires() {
        QTemporaryDir dir;
        ConfigManager c(dir.filePath("config.toml"));
        QSignalSpy spy(&c, &ConfigManager::configChanged);
        c.setTheme("latte");
        QVERIFY(spy.count() >= 1);
    }

    void brokenTomlRotates() {
        QTemporaryDir dir;
        const QString path = dir.filePath("config.toml");
        QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("this is not valid toml [[[\n");
        f.close();
        ConfigManager c(path);
        QCOMPARE(c.theme(), QString("default"));
        QVERIFY(QFile::exists(path));
        QDir d(dir.path());
        bool foundBroken = false;
        for (const auto &n : d.entryList({"config.toml.broken-*"}, QDir::Files))
            foundBroken = !n.isEmpty();
        QVERIFY(foundBroken);
    }
};
QTEST_MAIN(TestConfigManager)
#include "tst_configmanager.moc"
