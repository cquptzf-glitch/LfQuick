#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Ui/NavigationGuardController.h>
#include <LfQuick/Ui/QuickRuntime.h>

#include <QQmlEngine>
#include <QSignalSpy>
#include <QTest>

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

private:
    QString m_id;
};

class TestProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("guard-policy"); }
    bool canCreate(const PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &) override
    {
        return std::make_unique<TestPage>(descriptor.route);
    }
};

PageDescriptor descriptor(const QString &route)
{
    PageDescriptor value;
    value.route = route;
    value.provider = QStringLiteral("guard-policy");
    value.cachePolicy = PageCachePolicy::KeepAlive;
    value.fallbackPolicy = FallbackPolicy::None;
    return value;
}

void registerRoutes(QuickRuntime &runtime)
{
    QVERIFY(runtime.registerProvider(std::make_shared<TestProvider>()));
    QVERIFY(runtime.registerDescriptor(descriptor(QStringLiteral("/a"))));
    QVERIFY(runtime.registerDescriptor(descriptor(QStringLiteral("/b"))));
    QVERIFY(runtime.registerDescriptor(descriptor(QStringLiteral("/c"))));
    QVERIFY(runtime.push(QStringLiteral("/a")));
}

} // namespace

class NavigationGuardPolicyTest : public QObject
{
    Q_OBJECT

private slots:
    void keepFirstDoesNotOverwriteVisibleConfirmation()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        registerRoutes(runtime);

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        guard->state()->setReason(QStringLiteral("Unsaved changes"));
        guard->setPendingPolicy(NavigationGuardController::PendingPolicy::KeepFirst);

        QSignalSpy confirmations(guard, &NavigationGuardController::confirmationRequested);
        QSignalSpy dropped(guard, &NavigationGuardController::navigationRequestDropped);

        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(!runtime.push(QStringLiteral("/c")));

        QCOMPARE(guard->pendingRoute(), QStringLiteral("/b"));
        QCOMPARE(guard->queuedRequestCount(), 0);
        QCOMPARE(confirmations.count(), 1);
        QCOMPARE(dropped.count(), 1);
        QCOMPARE(dropped.at(0).at(1).toString(), QStringLiteral("/c"));
    }

    void replaceLatestExplicitlyChangesPendingRequest()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        registerRoutes(runtime);

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        guard->setPendingPolicy(NavigationGuardController::PendingPolicy::ReplaceLatest);

        QSignalSpy confirmations(guard, &NavigationGuardController::confirmationRequested);
        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(!runtime.push(QStringLiteral("/c")));

        QCOMPARE(guard->pendingRoute(), QStringLiteral("/c"));
        QCOMPARE(confirmations.count(), 2);
    }

    void queuePromotesNextRequestAfterCancel()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        registerRoutes(runtime);

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        guard->setPendingPolicy(NavigationGuardController::PendingPolicy::Queue);

        QSignalSpy confirmations(guard, &NavigationGuardController::confirmationRequested);
        QSignalSpy queued(guard, &NavigationGuardController::navigationRequestQueued);

        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(!runtime.push(QStringLiteral("/c")));
        QCOMPARE(guard->pendingRoute(), QStringLiteral("/b"));
        QCOMPARE(guard->queuedRequestCount(), 1);
        QCOMPARE(queued.count(), 1);

        guard->cancelPending();
        QCOMPARE(guard->pendingRoute(), QStringLiteral("/c"));
        QCOMPARE(guard->queuedRequestCount(), 0);
        QCOMPARE(confirmations.count(), 2);
    }

    void successfulConfirmationDropsRequestsFromOldOrigin()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        registerRoutes(runtime);

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        guard->setPendingPolicy(NavigationGuardController::PendingPolicy::Queue);

        QSignalSpy dropped(guard, &NavigationGuardController::navigationRequestDropped);
        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(!runtime.push(QStringLiteral("/c")));
        QCOMPARE(guard->queuedRequestCount(), 1);

        QVERIFY(guard->confirmPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/b"));
        QCOMPARE(guard->queuedRequestCount(), 0);
        QVERIFY(!guard->hasPending());
        QCOMPARE(dropped.count(), 1);
        QCOMPARE(dropped.at(0).at(1).toString(), QStringLiteral("/c"));
    }
};

QTEST_MAIN(NavigationGuardPolicyTest)
#include "tst_navigation_guard_policy.moc"
