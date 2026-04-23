#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "services/annotationstore.h"

class TestAnnotationStore : public QObject {
    Q_OBJECT
private:
    QString sidecarDir;
    AnnotationStore *store = nullptr;

    void makeStore() {
        delete store;
        store = new AnnotationStore();
        store->setStorageDir(sidecarDir);
    }

private slots:
    void init() {
        QTemporaryDir d;
        d.setAutoRemove(false);
        sidecarDir = d.path();
        makeStore();
        store->loadDocument("testhash");
    }
    void cleanup() { delete store; store = nullptr; }

    void addHighlightReturnsUuid() {
        QVariantList rects;
        rects.append(QVariant::fromValue(QRectF(10, 20, 30, 40)));
        const QString id = store->addHighlight(0, rects, QColor("#f9e2af"));
        QVERIFY(id.length() >= 36);
        QCOMPARE(store->rowCount(), 1);
    }

    void addAllFiveTypes() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        store->addHighlight(0, r, QColor("#f9e2af"));
        store->addUnderline(0, r, QColor("#a6e3a1"));
        store->addStrikeout(0, r, QColor("#f38ba8"));
        store->addStickyNote(0, QPointF(100, 100), QColor("#fab387"), "hello");
        QVariantList strokes;
        QVariantList pts; pts.append(QPointF(0, 0)); pts.append(QPointF(10, 10));
        strokes.append(pts);
        store->addInk(0, strokes, QColor("#89b4fa"), 2.0);
        QCOMPARE(store->rowCount(), 5);
    }

    void removeById() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        const QString id = store->addHighlight(0, r, QColor("#f9e2af"));
        store->remove(id);
        QCOMPARE(store->rowCount(), 0);
    }

    void persistRoundtrip() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(1, 2, 3, 4)));
        store->addHighlight(0, r, QColor("#f9e2af"));
        store->save();
        makeStore();
        store->loadDocument("testhash");
        QCOMPARE(store->rowCount(), 1);
        const auto annots = store->annotationsOnPage(0);
        QCOMPARE(annots.size(), 1);
    }

    void setColorUpdates() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        const QString id = store->addHighlight(0, r, QColor("#f9e2af"));
        QSignalSpy spy(store, &AnnotationStore::revisionChanged);
        store->setColor(id, QColor("#a6e3a1"));
        QVERIFY(spy.count() >= 1);
    }

    void annotationsOnPageFilters() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        store->addHighlight(0, r, QColor("#f9e2af"));
        store->addHighlight(3, r, QColor("#a6e3a1"));
        QCOMPARE(store->annotationsOnPage(0).size(), 1);
        QCOMPARE(store->annotationsOnPage(3).size(), 1);
        QCOMPARE(store->annotationsOnPage(7).size(), 0);
    }
};
QTEST_MAIN(TestAnnotationStore)
#include "tst_annotationstore.moc"
