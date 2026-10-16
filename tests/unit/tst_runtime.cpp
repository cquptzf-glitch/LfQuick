#include <LfQuick/Runtime/INavigationGuard.h>
#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageLifecycle.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <QtTest>

#include <memory>

namespace {

struct Counters {
    int created = 0;
    int activated = 0;
    int deactivated = 0;
    int suspended = 0;
    int resumed = 0;
    int destroyed = 0;
};

class FakePage final : public LfQuick::IPageInstance
{
public:
    FakePage(QString id, std::shared_ptr<Counters> counters)
        : m_id(std::move(id)), m_counters(std::move(counters)) {}

    QString pageId() const override { return m_id; }
    void activate() override { ++m_counters->activated; }
    void deactivate() override { ++m_counters->deactivated; }
    void suspend() override { ++m_counters->suspended; }
    void resume() override { ++m_counters->resumed; }
    void destroy() override { ++m_counters->destroyed; }

private:
    QString m_id;
    std::shared_ptr<Counters> m_counters;
};

class FakeProvider final : public LfQuick::IPageProvider
{
public:
    QString id() const override { return QStringLiteral("test"); }
    bool canCreate(const LfQuick::PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }

    std::unique_ptr<LfQuick::IPageInstance> create(
        const LfQuick::PageDescriptor &descriptor, const QVariantMap &) override
    {
        ++counters->created;
        return std::make_unique<FakePage>(descriptor.route, counters);
    }

    std::shared_ptr<Counters> counters = std::make_shared<Counters>();
};

class DenySamplesGuard final : public LfQuick::INavigationGuard
{
public:
    LfQuick::NavigationDecision evaluate(const LfQuick::NavigationContext &context) override
    {
        if (blocked && context.toRoute == QStringLiteral("/samples"))
            return LfQuick::NavigationDecision::deny(QStringLiteral("Unsaved result edits"));
        return LfQuick::NavigationDecision::allow();
    }

    bool blocked = true;
};

LfQuick::PageDescriptor page(QString route,
                             LfQuick::PageCachePolicy policy = LfQuick::PageCachePolicy::None)
{
    LfQuick::PageDescriptor descriptor;
    descriptor.route = std::move(route);
    descriptor.provider = QStringLiteral("test");
    descriptor.cachePolicy = policy;
    return descriptor;
}

} // namespace

class RuntimeTests : public QObject
{
    Q_OBJECT
private slots:
    void registersPage()
    {
        LfQuick::PageRegistry registry;
        auto descriptor = page(QStringLiteral("/dashboard"));
        descriptor.provider = QStringLiteral("quick");
        QVERIFY(registry.registerPage(descriptor));
        QVERIFY(!registry.registerPage(descriptor));
        QCOMPARE(registry.size(), 1);
        QVERIFY(registry.find(QStringLiteral("/dashboard")).has_value());
    }

    void lifecycleRejectsInvalidTransition()
    {
        LfQuick::PageLifecycle lifecycle;
        QVERIFY(!lifecycle.transitionTo(LfQuick::PageLifecycle::State::Active));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Ready));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Activating));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Active));
    }

    void nonePolicyRecreatesPageOnBack()
    {
        LfQuick::PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/dashboard"))));
        QVERIFY(pages.registerPage(page(QStringLiteral("/samples"))));

        auto provider = std::make_shared<FakeProvider>();
        LfQuick::PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::PageRouter router(&pages, &providers);
        QVERIFY(router.push({QStringLiteral("/dashboard"), {}, {}}).accepted);
        QVERIFY(router.push({QStringLiteral("/samples"), {}, {}}).accepted);

        QCOMPARE(provider->counters->created, 2);
        QCOMPARE(provider->counters->destroyed, 1);
        QVERIFY(router.back().accepted);

        QCOMPARE(router.currentRoute(), QStringLiteral("/dashboard"));
        QCOMPARE(provider->counters->created, 3);
        QCOMPARE(provider->counters->activated, 3);
        QCOMPARE(provider->counters->suspended, 0);
        QCOMPARE(provider->counters->resumed, 0);
        QCOMPARE(provider->counters->destroyed, 2);
    }

    void keepAliveResumesExistingPage()
    {
        LfQuick::PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/dashboard"),
                                        LfQuick::PageCachePolicy::KeepAlive)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/samples"))));

        auto provider = std::make_shared<FakeProvider>();
        LfQuick::PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::PageRouter router(&pages, &providers);
        QVERIFY(router.push({QStringLiteral("/dashboard"), {}, {}}).accepted);
        QVERIFY(router.push({QStringLiteral("/samples"), {}, {}}).accepted);
        QVERIFY(router.back().accepted);

        QCOMPARE(provider->counters->created, 2);
        QCOMPARE(provider->counters->suspended, 1);
        QCOMPARE(provider->counters->resumed, 1);
        QCOMPARE(provider->counters->destroyed, 1);
    }

    void lruEvictsOldestInactivePage()
    {
        LfQuick::PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/a"), LfQuick::PageCachePolicy::Lru)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/b"), LfQuick::PageCachePolicy::Lru)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/c"), LfQuick::PageCachePolicy::Lru)));

        auto provider = std::make_shared<FakeProvider>();
        LfQuick::PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::PageRouter router(&pages, &providers);
        router.setMaxRetainedLruPages(1);
        QSignalSpy evicted(&router, &LfQuick::PageRouter::cacheEntryEvicted);

        QVERIFY(router.push({QStringLiteral("/a"), {}, {}}).accepted);
        QVERIFY(router.push({QStringLiteral("/b"), {}, {}}).accepted);
        QVERIFY(router.push({QStringLiteral("/c"), {}, {}}).accepted);

        QCOMPARE(evicted.count(), 1);
        QCOMPARE(evicted.at(0).at(0).toString(), QStringLiteral("/a"));
        QCOMPARE(provider->counters->destroyed, 1);

        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/b"));
        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/a"));
        QCOMPARE(provider->counters->created, 4);
    }

    void navigationGuardBlocksRouteUntilReleased()
    {
        LfQuick::PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/dashboard"),
                                        LfQuick::PageCachePolicy::KeepAlive)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/samples"))));

        auto provider = std::make_shared<FakeProvider>();
        LfQuick::PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        auto guard = std::make_shared<DenySamplesGuard>();
        LfQuick::PageRouter router(&pages, &providers);
        QVERIFY(router.addGuard(guard));

        QSignalSpy blocked(&router, &LfQuick::PageRouter::navigationBlocked);
        QVERIFY(router.push({QStringLiteral("/dashboard"), {}, {}}).accepted);

        const auto denied = router.push({QStringLiteral("/samples"), {}, {}});
        QVERIFY(!denied.accepted);
        QCOMPARE(denied.error.domain(), QStringLiteral("navigation-guard"));
        QCOMPARE(router.currentRoute(), QStringLiteral("/dashboard"));
        QCOMPARE(blocked.count(), 1);

        guard->blocked = false;
        QVERIFY(router.push({QStringLiteral("/samples"), {}, {}}).accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/samples"));
    }
};

QTEST_APPLESS_MAIN(RuntimeTests)
#include "tst_runtime.moc"
