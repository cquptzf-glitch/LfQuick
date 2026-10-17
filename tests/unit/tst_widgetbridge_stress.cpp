#include <LfQuick/WidgetBridge/QuickSurface.h>
#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>

#include <QApplication>
#include <QDeadlineTimer>
#include <QDialog>
#include <QEventLoop>
#include <QKeyEvent>
#include <QLineEdit>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class WidgetBridgeStressTest : public QObject
{
    Q_OBJECT

private slots:
    void focusBridgeRoundTripsRepeatedly()
    {
        QWidget window;
        window.resize(560, 220);

        QLineEdit before(&window);
        before.setGeometry(10, 20, 140, 32);
        QQuickWidget surface(&window);
        surface.setGeometry(180, 20, 180, 120);
        surface.setFocusPolicy(Qt::StrongFocus);
        QLineEdit after(&window);
        after.setGeometry(390, 20, 140, 32);

        QQuickItem entry;
        QQuickItem exit;
        entry.setParentItem(surface.quickWindow()->contentItem());
        exit.setParentItem(surface.quickWindow()->contentItem());

        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, &after);
        bridge.setQuickBoundaryItems(&entry, &exit);
        QSignalSpy transfers(&bridge, &WidgetFocusBridge::focusTransferred);

        window.show();
        window.activateWindow();
        if (!QTest::qWaitForWindowFocused(&window, QDeadlineTimer(750)))
            QSKIP("Interactive window focus is unavailable on this test environment");

        constexpr int cycles = 30;
        for (int i = 0; i < cycles; ++i) {
            before.setFocus();
            QVERIFY(QTest::qWaitFor([&before] { return before.hasFocus(); }, 300));
            QTest::keyClick(&before, Qt::Key_Tab);
            QCOMPARE(surface.quickWindow()->activeFocusItem(), &entry);
            QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(&surface));

            QTest::keyClick(&surface, Qt::Key_Backtab);
            QVERIFY(QTest::qWaitFor([&before] { return before.hasFocus(); }, 300));

            after.setFocus();
            QVERIFY(QTest::qWaitFor([&after] { return after.hasFocus(); }, 300));
            QTest::keyClick(&after, Qt::Key_Backtab);
            QCOMPARE(surface.quickWindow()->activeFocusItem(), &exit);
            QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(&surface));

            QTest::keyClick(&surface, Qt::Key_Tab);
            QVERIFY(QTest::qWaitFor([&after] { return after.hasFocus(); }, 300));
        }

        QCOMPARE(transfers.count(), cycles * 4);
    }

    void focusBridgeGuardsRemainStableAcrossRepeatedToggles()
    {
        QWidget window;
        window.resize(480, 180);
        QLineEdit before(&window);
        before.setGeometry(10, 20, 120, 32);
        QQuickWidget surface(&window);
        surface.setGeometry(150, 20, 160, 100);
        surface.setFocusPolicy(Qt::StrongFocus);
        QQuickItem entry;
        entry.setParentItem(surface.quickWindow()->contentItem());

        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, nullptr);
        bridge.setQuickBoundaryItems(&entry, &entry);
        QSignalSpy transfers(&bridge, &WidgetFocusBridge::focusTransferred);

        window.show();
        window.activateWindow();
        if (!QTest::qWaitForWindowFocused(&window, QDeadlineTimer(750)))
            QSKIP("Interactive window focus is unavailable on this test environment");

        constexpr int cycles = 20;
        for (int i = 0; i < cycles; ++i) {
            const int baseline = transfers.count();

            bridge.setScannerIsolation(true);
            before.setFocus();
            QTest::keyClick(&before, Qt::Key_Tab);
            QCOMPARE(transfers.count(), baseline);

            bridge.setScannerIsolation(false);
            before.setFocus();
            QTest::keyClick(&before, Qt::Key_Tab);
            QCOMPARE(transfers.count(), baseline + 1);
            QTest::keyClick(&surface, Qt::Key_Backtab);
            QCOMPARE(transfers.count(), baseline + 2);
        }

        QDialog modal(&window);
        modal.setModal(true);
        modal.show();
        QVERIFY(QTest::qWaitFor([&] { return QApplication::activeModalWidget() == &modal; }, 500));

        const int beforeModalCount = transfers.count();
        QKeyEvent tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QApplication::sendEvent(&before, &tab);
        QCOMPARE(transfers.count(), beforeModalCount);

        modal.close();
        QVERIFY(QTest::qWaitFor([] { return QApplication::activeModalWidget() == nullptr; }, 500));
    }

    void quickSurfaceResizeAndMetricResetStress()
    {
        QQmlEngine engine;
        QuickSurface surface(&engine);
        surface.resize(420, 260);
        surface.show();
        surface.loadHost();

        QVERIFY(QTest::qWaitFor([&surface] {
            return surface.loadState() == QuickSurface::LoadState::Ready
                || surface.loadState() == QuickSurface::LoadState::Error;
        }, 1500));

        if (surface.loadState() == QuickSurface::LoadState::Error)
            QSKIP(qPrintable(QStringLiteral("QuickSurface host unavailable: %1").arg(surface.lastError())));

        QVERIFY(surface.isReady());
        QVERIFY(surface.loadDurationMs() >= 0);

        for (int i = 0; i < 120; ++i) {
            surface.resize(320 + (i % 17) * 7, 200 + (i % 13) * 5);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 2);
            QVERIFY(surface.rootObject());
            QCOMPARE(surface.rootObject()->size(), QSizeF(surface.size()));
        }

        surface.resetMetrics();
        QCOMPARE(surface.loadDurationMs(), -1);
        QCOMPARE(surface.firstFrameMs(), -1);
    }
};

QTEST_MAIN(WidgetBridgeStressTest)
#include "tst_widgetbridge_stress.moc"
