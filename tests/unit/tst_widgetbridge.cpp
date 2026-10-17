#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/WindowRouter.h>
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
#include <QWindow>

#include <memory>

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
        QVERIFY(instance->isValid());
        QCOMPARE(instance->pageId(), QStringLiteral("/legacy/results"));
        QCOMPARE(host.count(), 1);

        instance->activate();
        QCOMPARE(host.currentWidget()->objectName(), QStringLiteral("legacyResults"));
        QCOMPARE(qobject_cast<QLabel *>(host.currentWidget())->text(), QStringLiteral("Legacy Results"));

        instance->deactivate();
        instance->destroy();
    }

    void widgetPageProviderInvalidatesInstancesWhenHostChanges()
    {
        QStackedWidget firstHost;
        QStackedWidget secondHost;
        WidgetPageProvider provider(&firstHost);

        QVERIFY(provider.registerFactory(
            QStringLiteral("page"),
            [](QWidget *parent, const QVariantMap &) -> QWidget * {
                return new QLabel(QStringLiteral("Legacy"), parent);
            }));

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/legacy");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("page"));
        descriptor.cachePolicy = PageCachePolicy::KeepAlive;

        auto instance = provider.create(descriptor, {});
        QVERIFY(instance);
        QVERIFY(instance->isValid());
        QCOMPARE(firstHost.count(), 1);

        provider.setHost(&secondHost);
        QCOMPARE(provider.host(), &secondHost);
        QVERIFY(!instance->isValid());

        instance->destroy();
        QCOMPARE(firstHost.count(), 0);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void widgetPageProviderRejectsMissingHostBeforeFactoryRuns()
    {
        auto provider = std::make_shared<WidgetPageProvider>();
        bool factoryCalled = false;
        QVERIFY(provider->registerFactory(
            QStringLiteral("page"),
            [&](QWidget *, const QVariantMap &) -> QWidget * {
                factoryCalled = true;
                return new QLabel(QStringLiteral("Detached"));
            }));

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/legacy");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("page"));
        descriptor.cachePolicy = PageCachePolicy::KeepAlive;
        descriptor.fallbackPolicy = FallbackPolicy::None;

        QVERIFY(provider->canCreate(descriptor));
        QVERIFY(provider->create(descriptor, {}) == nullptr);
        QVERIFY(!factoryCalled);
        QCOMPARE(provider->lastErrorInfo().code, QStringLiteral("widget.host-missing"));

        PageRegistry pages;
        PageProviderRegistry providers;
        QVERIFY(pages.registerPage(descriptor));
        QVERIFY(providers.registerProvider(provider));
        PageRouter router(&pages, &providers);

        const RouteResult result = router.push({descriptor.route, {}, {}});
        QVERIFY(!result.accepted);
        const QVariantMap failure = result.error.details()
                                        .value(QStringLiteral("primaryFailure")).toMap();
        QCOMPARE(failure.value(QStringLiteral("code")).toString(),
                 QStringLiteral("widget.host-missing"));
        QVERIFY(!factoryCalled);
    }

    void widgetPageRouterRebuildsIntoNewHostOnBack()
    {
        QStackedWidget firstHost;
        QStackedWidget secondHost;
        PageRegistry pages;
        PageProviderRegistry providers;
        auto provider = std::make_shared<WidgetPageProvider>(&firstHost);
        int created = 0;

        QVERIFY(provider->registerFactory(
            QStringLiteral("page"),
            [&](QWidget *parent, const QVariantMap &parameters) -> QWidget * {
                ++created;
                return new QLabel(parameters.value(QStringLiteral("title")).toString(), parent);
            }));
        QVERIFY(providers.registerProvider(provider));

        PageDescriptor first;
        first.route = QStringLiteral("/first");
        first.provider = QStringLiteral("widget");
        first.source.insert(QStringLiteral("factory"), QStringLiteral("page"));
        first.cachePolicy = PageCachePolicy::KeepAlive;
        first.fallbackPolicy = FallbackPolicy::None;
        PageDescriptor second = first;
        second.route = QStringLiteral("/second");
        QVERIFY(pages.registerPage(first));
        QVERIFY(pages.registerPage(second));

        PageRouter router(&pages, &providers);
        QVERIFY(router.push({first.route,
                             {{QStringLiteral("title"), QStringLiteral("First")}}, {}}).accepted);
        QVERIFY(router.push({second.route,
                             {{QStringLiteral("title"), QStringLiteral("Second")}}, {}}).accepted);
        QCOMPARE(created, 2);
        QCOMPARE(firstHost.count(), 2);

        provider->setHost(&secondHost);
        QVERIFY(router.back().accepted);

        QCOMPARE(router.currentRoute(), first.route);
        QCOMPARE(created, 3);
        QCOMPARE(firstHost.count(), 0);
        QCOMPARE(secondHost.count(), 1);
        auto *label = qobject_cast<QLabel *>(secondHost.currentWidget());
        QVERIFY(label);
        QCOMPARE(label->text(), QStringLiteral("First"));
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
        QWidget *childWidget = nullptr;
        QWidget *receivedOwner = nullptr;

        QVERIFY(provider.registerFactory(
            QStringLiteral("tool"),
            [&](QWidget *owner, const QVariantMap &parameters) -> QWidget * {
                receivedOwner = owner;
                auto *widget = new QWidget(owner, Qt::Window);
                widget->setWindowTitle(parameters.value(QStringLiteral("title")).toString());
                if (!firstWidget)
                    firstWidget = widget;
                else
                    childWidget = widget;
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
        QVERIFY(firstWidget->parentWidget() == nullptr);

        const QString firstId = first->instanceId();
        auto second = provider.create(
            descriptor,
            {QStringLiteral("tool"), {{QStringLiteral("title"), QStringLiteral("Child")}}, firstId, {}, {}});
        QVERIFY(second != nullptr);
        QCOMPARE(receivedOwner, firstWidget);
        QVERIFY(childWidget);
        QVERIFY(childWidget->parentWidget() == nullptr);
        QVERIFY(childWidget->windowHandle());
        QVERIFY(firstWidget->windowHandle());
        QCOMPARE(childWidget->windowHandle()->transientParent(), firstWidget->windowHandle());
        QCOMPARE(childWidget->property("lfquickWindowOwnerInstanceId").toString(), firstId);

        QVERIFY(second->updateParameters({{QStringLiteral("mode"), QStringLiteral("updated")}}));
        second->close();
        first->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void widgetWindowProviderRejectsIncompatibleOwnerBeforeFactoryRuns()
    {
        WidgetWindowProvider provider;
        bool factoryCalled = false;
        QVERIFY(provider.registerFactory(
            QStringLiteral("tool"),
            [&](QWidget *, const QVariantMap &) -> QWidget * {
                factoryCalled = true;
                return new QWidget(nullptr, Qt::Window);
            }));

        WindowDescriptor descriptor;
        descriptor.id = QStringLiteral("tool");
        descriptor.provider = QStringLiteral("widget-window");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("tool"));

        auto instance = provider.create(
            descriptor,
            {descriptor.id, {}, QStringLiteral("foreign-provider-owner"), {}, {}});
        QVERIFY(instance == nullptr);
        QVERIFY(!factoryCalled);
        QCOMPARE(provider.lastErrorInfo().code, QStringLiteral("widget-window.owner-incompatible"));
        QCOMPARE(provider.lastErrorInfo().details.value(QStringLiteral("ownerInstanceId")).toString(),
                 QStringLiteral("foreign-provider-owner"));
    }

    void widgetWindowProviderDetachesFactoryQObjectParent()
    {
        WidgetWindowProvider provider;
        QWidget externalParent;
        QWidget *created = nullptr;
        QVERIFY(provider.registerFactory(
            QStringLiteral("tool"),
            [&](QWidget *, const QVariantMap &) -> QWidget * {
                created = new QWidget(&externalParent, Qt::Window);
                return created;
            }));

        WindowDescriptor descriptor;
        descriptor.id = QStringLiteral("tool");
        descriptor.provider = QStringLiteral("widget-window");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("tool"));

        auto instance = provider.create(descriptor, {descriptor.id, {}, {}, {}, {}});
        QVERIFY(instance != nullptr);
        QVERIFY(created);
        QVERIFY(created->parentWidget() == nullptr);

        instance->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void externallyDestroyedWidgetCleansWindowRouterRecord()
    {
        WindowProviderRegistry providers;
        auto provider = std::make_shared<WidgetWindowProvider>();
        QWidget *created = nullptr;

        QVERIFY(provider->registerFactory(
            QStringLiteral("tool"),
            [&](QWidget *, const QVariantMap &) -> QWidget * {
                created = new QWidget(nullptr, Qt::Window);
                return created;
            }));
        QVERIFY(providers.registerProvider(provider));

        WindowRouter router(&providers);
        WindowDescriptor descriptor;
        descriptor.id = QStringLiteral("tool");
        descriptor.provider = QStringLiteral("widget-window");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("tool"));
        descriptor.openPolicy = WindowOpenPolicy::MultipleInstances;
        QVERIFY(router.registerWindow(descriptor));

        QSignalSpy closed(&router, &WindowRouter::windowClosed);
        const WindowResult opened = router.open({descriptor.id, {}, {}, {}, {}});
        QVERIFY(opened.accepted);
        QVERIFY(created);
        QCOMPARE(router.count(), 1);
        QVERIFY(router.find(opened.instanceId).has_value());

        delete created;
        created = nullptr;

        QTRY_COMPARE(router.count(), 0);
        QCOMPARE(closed.count(), 1);
        QVERIFY(!router.find(opened.instanceId).has_value());
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

    void widgetOverlayBackendReconcilesDestroyedParent()
    {
        auto *window = new QWidget;
        window->resize(640, 360);

        OverlayCoordinator coordinator;
        WidgetOverlayBackend backend(window);
        coordinator.setBackend(&backend);

        const QString toastId = coordinator.showToast(
            QStringLiteral("Transient notification"),
            {{QStringLiteral("durationMs"), 0}});
        const QString dialogId = coordinator.requestDialog(
            QStringLiteral("Confirm"),
            QStringLiteral("Synthetic dialog"),
            {{QStringLiteral("buttons"), QStringList{QStringLiteral("ok"), QStringLiteral("cancel")}},
             {QStringLiteral("defaultButton"), QStringLiteral("ok")}});
        QVERIFY(!toastId.isEmpty());
        QVERIFY(!dialogId.isEmpty());
        QCOMPARE(coordinator.toastCount(), 1);
        QVERIFY(coordinator.dialogActive());
        QCOMPARE(coordinator.pendingDialogCount(), 1);

        QSignalSpy resolved(&coordinator, &OverlayCoordinator::dialogResolved);
        delete window;

        QCOMPARE(backend.parentWidget(), nullptr);
        QTRY_COMPARE(coordinator.toastCount(), 0);
        QTRY_VERIFY(!coordinator.dialogActive());
        QCOMPARE(coordinator.pendingDialogCount(), 0);
        QCOMPARE(resolved.count(), 1);
        QCOMPARE(resolved.at(0).at(0).toString(), dialogId);
        QCOMPARE(resolved.at(0).at(1).toString(), QStringLiteral("cancel"));
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