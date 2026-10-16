#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>
#include <LfQuick/WidgetBridge/WidgetOverlayBackend.h>
#include <LfQuick/WidgetBridge/WidgetPageProvider.h>
#include <LfQuick/WidgetBridge/WidgetWindowProvider.h>

#include <QApplication>
#include <QDeadlineTimer>
#include <QFrame>
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

    void widgetWindowProviderCreatesOwnedTopLevelWindows()
    {
        WidgetWindowProvider provider;
        QWidget *firstWidget = nullptr;
        QWidget *receivedOwner = nullptr;

        QVERIFY(provider.registerFactory(
            QStringLiteral("tool"),
            [&](QWidget *owner, const QVariantMap &parameters) -> QWidget * {
                receivedOwner = owner;
                auto *widget = new QWidget(owner, Qt::Window);
                widget->setWindowTitle(parameters.value(QStringLiteral("title")).toString());
                if (!firstWidget)
                    firstWidget = widget;
                return widget;
            }));

        WindowDescriptor descriptor;
        descriptor.id = QStringLiteral("tool");
        descriptor.provider = QStringLiteral("widget-window");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("tool"));

        auto first = provider.create(
            descriptor,
            {QStringLiteral("tool"), {{QStringLiteral("title"), QStringLiteral("Owner")}}, {}, {}, {}});
        QVERIFY(first != nullptr);
        QVERIFY(!first->instanceId().isEmpty());
        QVERIFY(firstWidget);
        QCOMPARE(firstWidget->windowTitle(), QStringLiteral("Owner"));
        QVERIFY(receivedOwner == nullptr);

        const QString firstId = first->instanceId();
        auto second = provider.create(
            descriptor,
            {QStringLiteral("tool"), {{QStringLiteral("title"), QStringLiteral("Child")}}, firstId, {}, {}});
        QVERIFY(second != nullptr);
        QCOMPARE(receivedOwner, firstWidget);

        QVERIFY(second->updateParameters({{QStringLiteral("mode"), QStringLiteral("updated")}}));
        second->close();
        first->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void widgetWindowProviderRejectsUnknownFactory()
    {
        WidgetWindowProvider provider;
        WindowDescriptor descriptor;
        descriptor.id = QStringLiteral("missing-window");
        descriptor.provider = QStringLiteral("widget-window");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("missing"));

        QVERIFY(!provider.canCreate(descriptor));
        QVERIFY(provider.create(descriptor, {descriptor.id, {}, {}, {}, {}}) == nullptr);
        QCOMPARE(provider.lastErrorInfo().code, QStringLiteral("widget-window.factory-not-found"));
    }

    void widgetOverlayBackendRendersCoordinatorToast()
    {
        QWidget window;
        window.resize(640, 360);

        OverlayCoordinator coordinator;
        WidgetOverlayBackend backend(&window);
        coordinator.setBackend(&backend);
        QVERIFY(coordinator.externalBackendActive());

        const QString toastId = coordinator.showToast(
            QStringLiteral("Legacy page notification"),
            {{QStringLiteral("severity"), QStringLiteral("success")},
             {QStringLiteral("durationMs"), 0}});
        QVERIFY(!toastId.isEmpty());
        QCOMPARE(coordinator.toastCount(), 1);

        auto *frame = window.findChild<QFrame *>(QStringLiteral("lfquickToast"));
        QVERIFY(frame);
        QVERIFY(frame->findChildren<QLabel *>().size() >= 2);

        QVERIFY(coordinator.dismissToast(toastId));
        QCOMPARE(coordinator.toastCount(), 0);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(!window.findChild<QFrame *>(QStringLiteral("lfquickToast")));
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
