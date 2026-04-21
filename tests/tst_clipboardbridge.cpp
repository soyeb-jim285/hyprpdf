#include <QTest>
#include <QGuiApplication>
#include <QClipboard>
#include "services/clipboardbridge.h"

class TestClipboardBridge : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        if (!qEnvironmentVariableIsSet("DISPLAY") &&
            !qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
            QSKIP("no display — clipboard test requires X11 or Wayland");
        }
    }

    void setTextRoundtrip() {
        ClipboardBridge b;
        b.setText("hello clipboard");
        QCOMPARE(QGuiApplication::clipboard()->text(), QString("hello clipboard"));
        QCOMPARE(b.text(), QString("hello clipboard"));
    }
};
QTEST_MAIN(TestClipboardBridge)
#include "tst_clipboardbridge.moc"
