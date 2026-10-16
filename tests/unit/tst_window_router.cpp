#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/WindowRouter.h>

#include <QtTest>

#include <memory>

namespace {

struct Counters {
    int created = 0;
    int activated = 0;
    int deactivated = 0;
    int closed = 0;
    int parameterUpdates = 0;
    int restored = 0;
};

class FakeWindow final : public LfQuick::IWindowInstance
{
public:
    FakeWindow(QString id, std::shared_ptr<Counters> counters)
        : m_id(std::move(id)), m_counters(std::move(counters)) {}

    QString instanceId() const override { return m_id; }
    void activate() override { ++m_counters->activated; }
    void deactivate() override { ++m_counters->deactivated; }
    bool canClose(QString *reason) const override
    {
        if (allowClose)
            return true;
        if (reason)
            *reason = QStringLiteral("unsaved-window-state");
        return false;
    }
    bool updateParameters(const QVariantMap &parameters) override
    {
        ++m_counters->parameterUpdates;
        lastParameters = parameters;
        return acceptParameters;
    }
    bool restoreState(const QVariantMap &state) override
    {
        ++m_counters->restored;
        lastRestoreState = state;
        return acceptRestoreState;
    }
    QVariantMap saveState() const override { return savedState; }
    void close() override { ++m_counters->closed; }

    bool allowClose = true;
    bool acceptParameters = true;
    bool acceptRestoreState = true;
    QVariantMap lastParameters;
    QVariantMap lastRestoreState;
    QVariantMap savedState;

private:
    QString m_id;
    std::shared_ptr<Counters> m_counters;
};

class FakeProvider final : public LfQuick::IWindowProvider
{
public:
    QString id() const override { return QStringLiteral("test-window"); }
    bool canCreate(const LfQuick::WindowDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }

    std::unique_ptr<LfQuick::IWindowInstance> create(
        const LfQuick::WindowDescriptor &,
        const LfQuick::WindowOpenRequest &) override
    {
        ++counters->created;
        if (failCreate)
            return {};

        const QString instanceId = QStringLiteral("window-%1").arg(counters->created);
        auto window = std::make_unique<FakeWindow>(instanceId, counters);
        lastCreated = window.get();
        createdWindows.push_back(window.get());
        return window;
    }

    LfQuick::WindowProviderError lastErrorInfo() const override
    {
        if (!failCreate)
            return {};
        return {QStringLiteral("create-failed"),
                QStringLiteral("fake provider create failed"),
                {{QStringLiteral("nativeCode"), 42}}};
    }

    bool failCreate = false;
    FakeWindow *lastCreated = nullptr;
    QList<FakeWindow *> createdWindows;
    std::shared_ptr<Counters> counters = std::make_shared<Counters>();
};

LfQuick::WindowDescriptor window(QString id,
                                 LfQuick::WindowOpenPolicy policy = LfQuick::WindowOpenPolicy::SingleInstance,
                                 LfQuick::OwnerClosePolicy ownerPolicy = LfQuick::OwnerClosePolicy::CloseWithOwner)
{
    LfQuick::WindowDescriptor descriptor;
    descriptor.id = std::move(id);
    descriptor.provider = QStringLiteral("test-window");
    descriptor.openPolicy = policy;
    descriptor.ownerClosePolicy = ownerPolicy;
    return descriptor;
}

} // namespace

class WindowRouterTests : public QObject
{
    Q_OBJECT

private slots:
    void singleInstanceReusesAndRefreshesParameters()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("settings"))));

        auto first = router.open({QStringLiteral("settings"), {{QStringLiteral("tab"), 1}}, {}, {}, {}});
        QVERIFY(first.accepted);
        QCOMPARE(router.count(), 1);

        auto second = router.open({QStringLiteral("settings"), {{QStringLiteral("tab"), 2}}, {}, {}, {}});
        QVERIFY(second.accepted);
        QCOMPARE(second.instanceId, first.instanceId);
        QCOMPARE(second.diagnostics.value(QStringLiteral("reused")).toBool(), true);
        QCOMPARE(provider->counters->created, 1);
        QCOMPARE(provider->counters->parameterUpdates, 1);
        QCOMPARE(router.find(first.instanceId)->parameters.value(QStringLiteral("tab")).toInt(), 2);
    }

    void multiInstanceCreatesIndependentWindows()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("result-detail"),
                                             LfQuick::WindowOpenPolicy::MultipleInstances)));

        const auto first = router.open({QStringLiteral("result-detail"), {}, {}, {}, {}});
        const auto second = router.open({QStringLiteral("result-detail"), {}, {}, {}, {}});
        QVERIFY(first.accepted);
        QVERIFY(second.accepted);
        QVERIFY(first.instanceId != second.instanceId);
        QCOMPARE(router.instances(QStringLiteral("result-detail")).size(), 2);
        QCOMPARE(provider->counters->created, 2);
    }

    void ownerClosePropagationIsAtomicWhenChildBlocks()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("owner"))));
        QVERIFY(router.registerWindow(window(QStringLiteral("child"),
                                             LfQuick::WindowOpenPolicy::MultipleInstances,
                                             LfQuick::OwnerClosePolicy::CloseWithOwner)));

        const auto owner = router.open({QStringLiteral("owner"), {}, {}, {}, {}});
        QVERIFY(owner.accepted);
        const auto child = router.open({QStringLiteral("child"), {}, owner.instanceId, {}, {}});
        QVERIFY(child.accepted);

        auto *childWindow = provider->lastCreated;
        QVERIFY(childWindow);
        childWindow->allowClose = false;

        const auto blocked = router.close(owner.instanceId);
        QVERIFY(!blocked.accepted);
        QCOMPARE(router.count(), 2);
        QVERIFY(router.find(owner.instanceId).has_value());
        QVERIFY(router.find(child.instanceId).has_value());
        QCOMPARE(provider->counters->closed, 0);

        childWindow->allowClose = true;
        QVERIFY(router.close(owner.instanceId).accepted);
        QCOMPARE(router.count(), 0);
        QCOMPARE(provider->counters->closed, 2);
    }

    void detachedChildSurvivesOwnerClose()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("owner"))));
        QVERIFY(router.registerWindow(window(QStringLiteral("tool"),
                                             LfQuick::WindowOpenPolicy::MultipleInstances,
                                             LfQuick::OwnerClosePolicy::Detach)));

        const auto owner = router.open({QStringLiteral("owner"), {}, {}, {}, {}});
        const auto tool = router.open({QStringLiteral("tool"), {}, owner.instanceId, {}, {}});
        QVERIFY(owner.accepted);
        QVERIFY(tool.accepted);

        QVERIFY(router.close(owner.instanceId).accepted);
        QCOMPARE(router.count(), 1);
        const auto info = router.find(tool.instanceId);
        QVERIFY(info.has_value());
        QVERIFY(info->ownerInstanceId.isEmpty());
    }

    void providerFailurePreservesStructuredDiagnostics()
    {
        auto provider = std::make_shared<FakeProvider>();
        provider->failCreate = true;
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("broken"))));

        const auto result = router.open({QStringLiteral("broken"), {}, {}, {}, {}});
        QVERIFY(!result.accepted);
        QCOMPARE(result.error.domain(), QStringLiteral("window-router"));
        const auto providerFailure = result.diagnostics.value(QStringLiteral("providerFailure")).toMap();
        QCOMPARE(providerFailure.value(QStringLiteral("code")).toString(), QStringLiteral("create-failed"));
        QCOMPARE(providerFailure.value(QStringLiteral("details")).toMap().value(QStringLiteral("nativeCode")).toInt(), 42);
    }

    void restoreStateRunsBeforeFirstActivation()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("restored"))));

        const auto result = router.open({QStringLiteral("restored"),
                                         {},
                                         {},
                                         {{QStringLiteral("geometry"), QStringLiteral("saved")}},
                                         {}});
        QVERIFY(result.accepted);
        QCOMPARE(provider->counters->restored, 1);
        QCOMPARE(provider->lastCreated->lastRestoreState.value(QStringLiteral("geometry")).toString(),
                 QStringLiteral("saved"));
    }

    void rejectsCyclicOwnershipOnSingleInstanceReuse()
    {
        auto provider = std::make_shared<FakeProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(window(QStringLiteral("a"))));
        QVERIFY(router.registerWindow(window(QStringLiteral("b"))));

        const auto a = router.open({QStringLiteral("a"), {}, {}, {}, {}});
        const auto b = router.open({QStringLiteral("b"), {}, a.instanceId, {}, {}});
        QVERIFY(a.accepted);
        QVERIFY(b.accepted);

        const auto cycle = router.open({QStringLiteral("a"), {}, b.instanceId, {}, {}});
        QVERIFY(!cycle.accepted);
        QCOMPARE(cycle.error.code(), 13);
        QCOMPARE(router.find(a.instanceId)->ownerInstanceId, QString());
    }
};

QTEST_MAIN(WindowRouterTests)
#include "tst_window_router.moc"
