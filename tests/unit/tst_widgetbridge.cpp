#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>
#include <LfQuick/WidgetBridge/WidgetPageProvider.h>

#include <QApplication>
#include <QDeadlineTimer>
#include <QLabel>
#include <QLineEdit>
#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTest>

using namespace LfQuick;

class WidgetBridgeTest : public QObject
{
    Q_OBJECT

private slots:
    void widgetProviderCreatesLegacyPageInSharedStack()
    {
        QStackedWidget host;
        WidgetPageProvider provider(&host);

        QVERIFY(provider.registerFactory(
            QStringLiteral("legacy-results"),
            [](QWidget *parent, const QVariantMap &parameters) -> QWidget * {
                auto *label = new QLabel(parameters.value(QStringLiteral("title")).toString(), parent);
                label->setObjectName(QStringLiteral("legacyResults"));
                return label;
            }));

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/legacy/results");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("legacy-results"));
        descriptor.cachePolicy = PageCachePolicy::KeepAlive;

        auto instance = provider.create(
            descriptor,
            {{QStringLiteral("title"), QStringLiteral("Legacy Results")}});

        QVERIFY(instance != nullptr);
        QCOMPARE(instance->pageId(), QStringLiteral("/legacy/results"));
        QCOMPARE(host.count(), 1);

        instance->activate();
        QCOMPARE(host.currentWidget()->objectName(), QStringLiteral("legacyResults"));
        QCOMPARE(qobject_cast<QLabel *>(host.currentWidget())->text(), QStringLiteral("Legacy Results"));

        instance->deactivate();
        instance->destroy();
    }

    void widgetProviderRejectsUnknownFactory()
    {
        QStackedWidget host;
        WidgetPageProvider provider(&host);

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/missing");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("missing"));

        QVERIFY(!provider.canCreate(descriptor));
        QVERIFY(provider.create(descriptor, {}) == nullptr);
        QVERIFY(provider.lastError().contains(QStringLiteral("missing")));
    }

    void focusBridgeTransfersWidgetTabIntoQuickBoundary()
    {
        QWidget window;
        window.resize(480, 180);
        QLineEdit before(&window);
        before.setGeometry(10, 20, 120, 32);
        QQuickWidget surface(&window);
        surface.setGeometry(150, 20, 160, 100);
        surface.setFocusPolicy(Qt::StrongFocus);
        QQuickItem entry;
        QLineEdit after(&window);
        after.setGeometry(330, 20, 120, 32);

        window.show();
        window.activateWindow();
        if (!QTest::qWaitForWindowFocused(&window, QDeadlineTimer(500)))
            QSKIP("Interactive window focus is unavailable on this test runner");

        entry.setParentItem(surface.quickWindow()->contentItem());
        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, &after);
        bridge.setQuickBoundaryItems(&entry, &entry);
        QSignalSpy transferred(&bridge, &WidgetFocusBridge::focusTransferred);

        before.setFocus();
        QVERIFY(QTest::qWaitFor([&before] { return before.hasFocus(); }, 500));
        QTest::keyClick(&before, Qt::Key_Tab);

        QCOMPARE(transferred.count(), 1);
        QCOMPARE(transferred.at(0).at(0).toString(), QStringLiteral("widget-to-quick"));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(&surface));
    }

    void focusBridgeScannerIsolationSuppressesTransfer()
    {
        QWidget window;
        window.resize(480, 180);
        QLineEdit before(&window);
        before.setGeometry(10, 20, 120, 32);
        QQuickWidget surface(&window);
        surface.setGeometry(150, 20, 160, 100);
        surface.setFocusPolicy(Qt::StrongFocus);
        QQuickItem entry;
        QLineEdit after(&window);
        after.setGeometry(330, 20, 120, 32);

        window.show();
        window.activateWindow();
        if (!QTest::qWaitForWindowFocused(&window, QDeadlineTimer(500)))
            QSKIP("Interactive window focus is unavailable on this test runner");

        entry.setParentItem(surface.quickWindow()->contentItem());
        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, &after);
        bridge.setQuickBoundaryItems(&entry, &entry);
        bridge.setScannerIsolation(true);
        QSignalSpy transferred(&bridge, &WidgetFocusBridge::focusTransferred);

        before.setFocus();
        QVERIFY(QTest::qWaitFor([&before] { return before.hasFocus(); }, 500));
        QTest::keyClick(&before, Qt::Key_Tab);

        QCOMPARE(transferred.count(), 0);
    }
};

QTEST_MAIN(WidgetBridgeTest)
#include "tst_widgetbridge.moc"
