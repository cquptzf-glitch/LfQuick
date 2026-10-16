#include <LfQuick/Ui/NavigationGuardController.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QuickRuntime.h>

#include <LfQuick/Core/OperationHandle.h>
#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>

#include <QTest>
#include <QQmlEngine>
#include <QUrl>

#include <memory>

using namespace LfQuick;

namespace {

class TestPage final : public IPageInstance
{
public:
    explicit TestPage(QString id) : m_id(std::move(id)) {}
    QString pageId() const override { return m_id; }
    void activate() override {}
    void deactivate() override {}
    void suspend() override {}
    void resume() override {}
    void destroy() override {}

private:
    QString m_id;
};

class TestProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("guard-test"); }
    bool canCreate(const PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override
    {
        lastParameters = parameters;
        return std::make_unique<TestPage>(descriptor.route);
    }

    QVariantMap lastParameters;
};

PageDescriptor guardTestPage(const QString &route)
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("guard-test");
    descriptor.cachePolicy = PageCachePolicy::KeepAlive;
    descriptor.fallbackPolicy = FallbackPolicy::None;
    return descriptor;
}

} // namespace

class UiTest : public QObject
{
    Q_OBJECT

private slots:
    void busyOverlayFollowsOperation()
    {
        OverlayCoordinator coordinator;
        OperationHandle operation;
        operation.setMessage(QStringLiteral("Calibrating"));
        operation.setCancelable(true);
        operation.setProgress(0.25);

        const QString token = coordinator.bindOperation(&operation);
        QVERIFY(!token.isEmpty());
        QVERIFY(coordinator.busy());
        QCOMPARE(coordinator.busyCount(), 1);
        QCOMPARE(coordinator.busyMessage(), QStringLiteral("Calibrating"));
        QCOMPARE(coordinator.busyProgress(), 0.25);
        QVERIFY(coordinator.busyCancelable());

        operation.setProgress(0.75);
        QCOMPARE(coordinator.busyProgress(), 0.75);

        operation.setState(OperationHandle::State::Succeeded);
        QVERIFY(!coordinator.busy());
    }

    void qmlErrorCollectorKeepsStructuredDiagnostics()
    {
        QmlErrorCollector collector;
        QSignalSpy recorded(&collector, &QmlErrorCollector::errorRecorded);

        const QUrl source(QStringLiteral("qrc:/BrokenPage.qml"));
        collector.record(QStringLiteral("/results"), source,
                         QStringLiteral("Type ResultPage unavailable"),
                         QStringLiteral("primary"));

        QCOMPARE(collector.count(), 1);
        QCOMPARE(recorded.count(), 1);
        const auto entry = collector.lastError();
        QCOMPARE(entry.value(QStringLiteral("route")).toString(), QStringLiteral("/results"));
        QCOMPARE(entry.value(QStringLiteral("source")).toUrl(), source);
        QCOMPARE(entry.value(QStringLiteral("stage")).toString(), QStringLiteral("primary"));
        QVERIFY(entry.value(QStringLiteral("message")).toString().contains(QStringLiteral("ResultPage")));

        collector.clear();
        QCOMPARE(collector.count(), 0);
    }

    void quickRuntimeRejectsUnknownRouteAndCanDisableFallback()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);

        QVERIFY(runtime.registerPage(
            QStringLiteral("/missing-source"),
            QUrl(QStringLiteral("file:///definitely/not/a/page.qml")),
            static_cast<int>(PageCachePolicy::KeepAlive),
            false));

        QVERIFY(!runtime.push(QStringLiteral("/unknown")));
        QVERIFY(!runtime.push(QStringLiteral("/missing-source")));
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.errorCollector()->count(), 1);
    }

    void navigationGuardConfirmRetriesExactlyOnce()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        QVERIFY(guard);
        guard->state()->setDirty(true);
        guard->state()->setReason(QStringLiteral("Unsaved result edits"));
        guard->state()->setConfirmRequired(true);

        QSignalSpy confirmation(guard, &NavigationGuardController::confirmationRequested);
        const QVariantMap parameters{{QStringLiteral("sampleId"), QStringLiteral("S-1001")}};
        QVERIFY(!runtime.push(QStringLiteral("/b"), parameters));
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
        QVERIFY(guard->hasPending());
        QCOMPARE(guard->pendingRoute(), QStringLiteral("/b"));
        QCOMPARE(guard->pendingParameters(), parameters);
        QCOMPARE(confirmation.count(), 1);

        QVERIFY(guard->confirmPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/b"));
        QCOMPARE(provider->lastParameters, parameters);
        QVERIFY(!guard->hasPending());
        QCOMPARE(confirmation.count(), 1); // retry bypasses this guard once
    }

    void navigationGuardCancelKeepsCurrentRoute()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(guard->hasPending());

        guard->cancelPending();
        QVERIFY(!guard->hasPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
    }

    void navigationGuardHardBlockCannotBeConfirmed()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        guard->state()->setBusy(true);
        guard->state()->setConfirmRequired(false);
        guard->state()->setReason(QStringLiteral("Calibration is running"));
        QSignalSpy hardBlocked(guard, &NavigationGuardController::hardBlocked);

        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QCOMPARE(hardBlocked.count(), 1);
        QVERIFY(guard->hasPending());
        QVERIFY(!guard->confirmPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
    }
};

QTEST_MAIN(UiTest)
#include "tst_ui.moc"
