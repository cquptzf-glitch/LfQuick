#include <LfQuick/Runtime/IWindowBacking.h>
#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/WindowRouter.h>

#include <QtTest>

#include <memory>
#include <utility>

namespace {

struct NativeSurface {
    int marker = 42;
};

struct Counters {
    int created = 0;
    int recreated = 0;
    int activated = 0;
    int closed = 0;
    int restored = 0;
};

class FakeBacking final : public LfQuick::IWindowBacking
{
public:
    FakeBacking(QString id, std::shared_ptr<NativeSurface> surface)
        : m_id(std::move(id)), m_surface(std::move(surface)) {}

    QString providerId() const override { return QStringLiteral("reload-window"); }
    LfQuick::WindowInstanceId instanceId() const override { return m_id; }

    std::shared_ptr<NativeSurface> takeSurface()
    {
        return std::exchange(m_surface, {});
    }

    const std::shared_ptr<NativeSurface> &surface() const { return m_surface; }

private:
    QString m_id;
    std::shared_ptr<NativeSurface> m_surface;
};

class FakeReloadWindow final : public LfQuick::IWindowInstance
{
public:
    FakeReloadWindow(QString id,
                     std::shared_ptr<NativeSurface> surface,
                     std::shared_ptr<Counters> counters,
                     bool rejectRestore = false)
        : m_id(std::move(id))
        , m_surface(std::move(surface))
        , m_counters(std::move(counters))
        , m_rejectRestore(rejectRestore)
    {
    }

    QString instanceId() const override { return m_id; }
    void activate() override { ++m_counters->activated; }
    void deactivate() override {}
    void close() override
    {
        ++m_counters->closed;
        m_surface.reset();
    }

    bool updateParameters(const QVariantMap &parameters) override
    {
        m_parameters = parameters;
        return true;
    }

    QVariantMap saveState() const override
    {
        return {{QStringLiteral("value"), 73}};
    }

    bool restoreState(const QVariantMap &state) override
    {
        ++m_counters->restored;
        m_restoredState = state;
        return !m_rejectRestore;
    }

    bool canReleaseBacking() const noexcept override
    {
        return bool(m_surface);
    }

    std::unique_ptr<LfQuick::IWindowBacking> releaseBacking() override
    {
        if (!m_surface)
            return {};
        auto surface = std::exchange(m_surface, {});
        return std::make_unique<FakeBacking>(m_id, std::move(surface));
    }

    bool canAdoptBacking(const LfQuick::IWindowBacking &backing) const noexcept override
    {
        const auto *typed = dynamic_cast<const FakeBacking *>(&backing);
        return !m_surface && typed && typed->instanceId() == m_id && typed->surface();
    }

    bool adoptBacking(std::unique_ptr<LfQuick::IWindowBacking> &backing) override
    {
        if (!backing || !canAdoptBacking(*backing))
            return false;
        auto *typed = dynamic_cast<FakeBacking *>(backing.get());
        m_surface = typed->takeSurface();
        backing.reset();
        return bool(m_surface);
    }

    const std::shared_ptr<NativeSurface> &surface() const { return m_surface; }
    const QVariantMap &restoredState() const { return m_restoredState; }

private:
    QString m_id;
    std::shared_ptr<NativeSurface> m_surface;
    std::shared_ptr<Counters> m_counters;
    QVariantMap m_parameters;
    QVariantMap m_restoredState;
    bool m_rejectRestore = false;
};

class ReloadProvider final : public LfQuick::IWindowProvider
{
public:
    QString id() const override { return QStringLiteral("reload-window"); }

    bool canCreate(const LfQuick::WindowDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }

    std::unique_ptr<LfQuick::IWindowInstance> create(
        const LfQuick::WindowDescriptor &,
        const LfQuick::WindowOpenRequest &) override
    {
        ++counters->created;
        auto surface = std::make_shared<NativeSurface>();
        originalSurface = surface;
        auto window = std::make_unique<FakeReloadWindow>(
            QStringLiteral("stable-instance"), surface, counters);
        lastWindow = window.get();
        return window;
    }

    bool canAdoptBacking(const LfQuick::WindowDescriptor &descriptor,
                         const LfQuick::IWindowBacking &backing) const override
    {
        const auto *typed = dynamic_cast<const FakeBacking *>(&backing);
        return descriptor.provider == id()
            && typed
            && typed->providerId() == id()
            && typed->instanceId() == QStringLiteral("stable-instance")
            && typed->surface();
    }

    std::unique_ptr<LfQuick::IWindowInstance> createWithBacking(
        const LfQuick::WindowDescriptor &descriptor,
        const LfQuick::WindowOpenRequest &,
        std::unique_ptr<LfQuick::IWindowBacking> &backing) override
    {
        ++counters->recreated;
        if (failRecreate)
            return {};
        if (!backing || !canAdoptBacking(descriptor, *backing))
            return {};

        auto *typed = dynamic_cast<FakeBacking *>(backing.get());
        const QString instanceId = typed->instanceId();
        auto surface = typed->takeSurface();
        backing.reset();

        auto window = std::make_unique<FakeReloadWindow>(
            instanceId, std::move(surface), counters, rejectReplacementRestore);
        lastWindow = window.get();
        return window;
    }

    LfQuick::WindowProviderError lastErrorInfo() const override
    {
        if (!failRecreate)
            return {};
        return {QStringLiteral("reload-failed"),
                QStringLiteral("synthetic reload creation failure"), {}};
    }

    std::shared_ptr<Counters> counters = std::make_shared<Counters>();
    std::shared_ptr<NativeSurface> originalSurface;
    FakeReloadWindow *lastWindow = nullptr;
    bool failRecreate = false;
    bool rejectReplacementRestore = false;
};

LfQuick::WindowDescriptor descriptor()
{
    LfQuick::WindowDescriptor result;
    result.id = QStringLiteral("reloadable");
    result.provider = QStringLiteral("reload-window");
    return result;
}

} // namespace

class WindowReloadTests : public QObject
{
    Q_OBJECT

private slots:
    void preservesBackingIdentityAndState()
    {
        auto provider = std::make_shared<ReloadProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor()));
        const auto opened = router.open({QStringLiteral("reloadable"),
                                         {{QStringLiteral("mode"), QStringLiteral("a")}},
                                         {}, {}, {}});
        QVERIFY(opened.accepted);
        QCOMPARE(opened.instanceId, QStringLiteral("stable-instance"));
        const NativeSurface *surfaceAddress = provider->originalSurface.get();
        QVERIFY(surfaceAddress);

        const auto reloaded = router.reloadInstance(opened.instanceId);
        QVERIFY(reloaded.accepted);
        QCOMPARE(reloaded.instanceId, opened.instanceId);
        QCOMPARE(reloaded.diagnostics.value(QStringLiteral("backingPreserved")).toBool(), true);
        QCOMPARE(router.count(), 1);
        QCOMPARE(router.activeInstanceId(), opened.instanceId);
        QVERIFY(provider->lastWindow);
        QCOMPARE(provider->lastWindow->surface().get(), surfaceAddress);
        QCOMPARE(provider->lastWindow->restoredState().value(QStringLiteral("value")).toInt(), 73);
        QCOMPARE(provider->counters->recreated, 1);
        QCOMPARE(provider->counters->activated, 2); // initial open + restored active state
    }

    void createFailureRollsBackingBackToOldInstance()
    {
        auto provider = std::make_shared<ReloadProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor()));
        const auto opened = router.open({QStringLiteral("reloadable"), {}, {}, {}, {}});
        QVERIFY(opened.accepted);
        const NativeSurface *surfaceAddress = provider->originalSurface.get();

        provider->failRecreate = true;
        const auto result = router.reloadInstance(opened.instanceId);
        QVERIFY(!result.accepted);
        QCOMPARE(router.count(), 1);
        QCOMPARE(router.activeInstanceId(), opened.instanceId);
        QVERIFY(router.find(opened.instanceId).has_value());

        provider->failRecreate = false;
        const auto retry = router.reloadInstance(opened.instanceId);
        QVERIFY(retry.accepted);
        QVERIFY(provider->lastWindow);
        QCOMPARE(provider->lastWindow->surface().get(), surfaceAddress);
    }

    void restoreFailureRecoversBackingFromCandidateAndRollsBack()
    {
        auto provider = std::make_shared<ReloadProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor()));
        const auto opened = router.open({QStringLiteral("reloadable"), {}, {}, {}, {}});
        QVERIFY(opened.accepted);
        const NativeSurface *surfaceAddress = provider->originalSurface.get();

        provider->rejectReplacementRestore = true;
        const auto failed = router.reloadInstance(opened.instanceId);
        QVERIFY(!failed.accepted);
        QCOMPARE(router.count(), 1);
        QCOMPARE(router.activeInstanceId(), opened.instanceId);

        provider->rejectReplacementRestore = false;
        const auto retry = router.reloadInstance(opened.instanceId);
        QVERIFY(retry.accepted);
        QCOMPARE(provider->lastWindow->surface().get(), surfaceAddress);
    }
};

QTEST_MAIN(WindowReloadTests)
#include "tst_window_reload.moc"
