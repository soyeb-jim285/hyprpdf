#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJSEngine>
#include <QJSValue>
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

    void addInkAcceptsPointMapsLikeQml() {
        QVariantList stroke;
        stroke.append(QVariantMap{{"x", 1.5}, {"y", 2.5}});
        stroke.append(QVariantMap{{"x", 6.0}, {"y", 8.0}});

        QVariantList strokes;
        strokes.append(stroke);

        const QString id = store->addInk(4, strokes, QColor("#89b4fa"), 3.0);
        QVERIFY(!id.isEmpty());

        const auto annots = store->annotationsOnPage(4);
        QCOMPARE(annots.size(), 1);

        const auto annot = annots[0].toMap();
        const auto returnedStrokes = annot.value("strokes").toList();
        QCOMPARE(returnedStrokes.size(), 1);

        const auto returnedPoints = returnedStrokes[0].toList();
        QCOMPARE(returnedPoints.size(), 2);
        QCOMPARE(returnedPoints[0].toPointF(), QPointF(1.5, 2.5));
        QCOMPARE(returnedPoints[1].toPointF(), QPointF(6.0, 8.0));
        QCOMPARE(annot.value("strokeWidth").toDouble(), 3.0);
    }

    void addInkRejectsMalformedStroke() {
        QVariantList malformedStroke;
        malformedStroke.append(QVariantMap{{"x", 1.0}});

        QVariantList strokes;
        strokes.append(malformedStroke);

        const QString id = store->addInk(0, strokes, QColor("#89b4fa"), 2.0);
        QVERIFY(id.isEmpty());
        QCOMPARE(store->rowCount(), 0);
    }

    void addInkAcceptsNestedJsArrayFromQml() {
        QJSEngine engine;
        QJSEngine::setObjectOwnership(store, QJSEngine::CppOwnership);
        engine.globalObject().setProperty("annotationStore", engine.newQObject(store));

        QJSValue currentStroke = engine.newArray(3);
        currentStroke.setProperty(0, engine.toScriptValue(QPointF(2, 3)));
        currentStroke.setProperty(1, engine.toScriptValue(QPointF(9, 11)));
        currentStroke.setProperty(2, engine.toScriptValue(QPointF(18, 21)));

        QJSValue strokes = engine.newArray(1);
        strokes.setProperty(0, currentStroke);
        engine.globalObject().setProperty("strokesArg", strokes);

        const QJSValue result = engine.evaluate(
            "annotationStore.addInk(2, strokesArg, '#89b4fa', 2.5)");
        QVERIFY2(!result.isError(), qPrintable(result.toString()));

        const QString id = result.toString();
        QVERIFY(!id.isEmpty());

        const auto annots = store->annotationsOnPage(2);
        QCOMPARE(annots.size(), 1);
        const auto annot = annots[0].toMap();
        const auto returnedStrokes = annot.value("strokes").toList();
        QCOMPARE(returnedStrokes.size(), 1);
        QCOMPARE(returnedStrokes[0].toList().size(), 3);
    }

    void undoReversesAdd() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        store->addHighlight(0, r, QColor("#f9e2af"));
        QCOMPARE(store->rowCount(), 1);
        store->undo();
        QCOMPARE(store->rowCount(), 0);
    }

    void redoReappliesAdd() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        store->addHighlight(0, r, QColor("#f9e2af"));
        store->undo();
        store->redo();
        QCOMPARE(store->rowCount(), 1);
    }

    void undoDepthFiftyEvictsOldest() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        for (int i = 0; i < 60; ++i)
            store->addHighlight(0, r, QColor("#f9e2af"));
        QCOMPARE(store->rowCount(), 60);
        int undoable = 0;
        while (store->canUndo()) { store->undo(); ++undoable; if (undoable > 100) break; }
        QCOMPARE(undoable, 50);
        QCOMPARE(store->rowCount(), 10);
    }

    void exportMarkdownContainsTypeAndColor() {
        QVariantList r; r.append(QVariant::fromValue(QRectF(0, 0, 10, 10)));
        store->addHighlight(2, r, QColor("#f9e2af"));
        store->addStickyNote(2, QPointF(50, 50), QColor("#fab387"), "my note");
        QTemporaryDir d;
        const QString mdPath = d.path() + "/out.md";
        store->exportMarkdown(mdPath);
        QFile f(mdPath);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QString content = QString::fromUtf8(f.readAll());
        QVERIFY(content.contains("## Page 3"));
        QVERIFY(content.contains("**Highlight**"));
        QVERIFY(content.contains("#f9e2af"));
        QVERIFY(content.contains("**Note**"));
        QVERIFY(content.contains("my note"));
    }
};
QTEST_MAIN(TestAnnotationStore)
#include "tst_annotationstore.moc"
