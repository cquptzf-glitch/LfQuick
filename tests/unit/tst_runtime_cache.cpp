#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace LfQuick;

namespace {

struct Stats {
    int created = 0;
    int destroyed = 0;
    int parameterUpdates = 0;
    QVariantMap lastParameters;
};

class CachePage final : public IPageInstance
{
public:
    CachePage(QString id, std::shared_ptr<Stats> stats)
        : m_id(std::move(id)), m_stats(std::move(stats)) {}

    QString pageId() const override { return m_id; }
    void activate() override {}
    void deactivate() override {}
    void suspend() override {}
    void resume() override {}
    bool updateParameters(const QVariantMap &parameters) override
    {
        ++m_stats->parameterUpdates;
        m_stats->lastParameters = parameters;
        return true;
    }
    void destroy() override { ++m_stats->destroyed; }

private:
    QString m_id;
    std::shared_ptr<Stats> m_stats;
};

class CacheProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("cache-test"); }
    bool canCreate(const PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &) override
    {
        ++stats->created;
        return std::make_unique<CachePage>(descriptor.route, stats);
    }

    std::shared_ptr<Stats> stats = std::make_shared<Stats>();
};

class StructuredFailureProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("structured-failure"); }
    bool canCreate(const PageDescriptor &) const override { return true; }
    std::unique_ptr<IPageInstance> create(const PageDescriptor &,
                                           const QVariantMap &) override
    {
        return {};
    }
    QString lastError() const override { return QStringLiteral("legacy failure text"); }
    PageProviderError lastErrorInfo() const override
    {
        return {
            QStringLiteral("test.create-failed"),
            QStringLiteral("structured create failure"),
            {{QStringLiteral("source"), QStringLiteral("unit-test")},
             {QStringLiteral("retryable"), false}}
        };
    }
};

PageDescriptor page(const QString &route, PageCachePolicy policy, qint64 weight = 1)
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("cache-test");
    descriptor.cachePolicy = policy;
    descriptor.cacheWeight = weight;
    descriptor.fallbackPolicy = FallbackPolicy::None;
    return descriptor;
}

} // namespace

class RuntimeCacheTest : public QObject
{
    Q_OBJECT

private slots:
    void singletonIsRouteGlobalAcrossHistoryEntries()
    {
        PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/singleton"), PageCachePolicy::Singleton)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/other"), PageCachePolicy::KeepAlive)));

        auto provider = std::make_shared<CacheProvider>();
        PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        PageRouter router(&pages, &providers);
        QSignalSpy created(&router, &PageRouter::singletonCreated);
        QSignalSpy reused(&router, &PageRouter::singletonReused);

        QVERIFY(router.push({QStringLiteral("/singleton"),
                             {{QStringLiteral("recordId"), 1}}, {}}).accepted);
        QCOMPARE(router.singletonCount(), 1);
        QCOMPARE(created.count(), 1);
        QCOMPARE(provider->stats->created, 1);

        QVERIFY(router.push({QStringLiteral("/other"), {}, {}}).accepted);
        QCOMPARE(provider->stats->created, 2);

        QVERIFY(router.push({QStringLiteral("/singleton"),
                             {{QStringLiteral("recordId"), 2}}, {}}).accepted);
        QCOMPARE(provider->stats->created, 2);
        QCOMPARE(router.singletonCount(), 1);
        QCOMPARE(reused.count(), 1);
        QVERIFY(provider->stats->parameterUpdates >= 1);
        QCOMPARE(provider->stats->lastParameters.value(QStringLiteral("recordId")).toInt(), 2);

        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/other"));
        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/singleton"));
        QCOMPARE(provider->stats->created, 2);

        router.reset();
        QCOMPARE(router.singletonCount(), 0);
        // Singleton is destroyed once by the global cache; the KeepAlive page
        // is destroyed independently.
        QCOMPARE(provider->stats->destroyed, 2);
    }

    void weightedLruEvictsOldestUntilBothBudgetsFit()
    {
        PageRegistry pages;
        QVERIFY(pages.registerPage(page(QStringLiteral("/a"), PageCachePolicy::Lru, 3)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/b"), PageCachePolicy::Lru, 2)));
        QVERIFY(pages.registerPage(page(QStringLiteral("/c"), PageCachePolicy::Lru, 1)));

        auto provider = std::make_shared<CacheProvider>();
        PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        PageRouter router(&pages, &providers);
        router.setMaxRetainedLruPages(10);
        router.setMaxRetainedLruWeight(3);
        QSignalSpy evicted(&router, &PageRouter::cacheEntryEvicted);

        QVERIFY(router.push({QStringLiteral("/a"), {}, {}}).accepted);
        QVERIFY(router.push({QStringLiteral("/b"), {}, {}}).accepted);
        QCOMPARE(router.retainedLruWeight(), 3);

        QVERIFY(router.push({QStringLiteral("/c"), {}, {}}).accepted);
        // Inactive /a(3) + /b(2) exceeds weight 3, so oldest /a is evicted.
        QCOMPARE(evicted.count(), 1);
        QCOMPARE(evicted.at(0).at(0).toString(), QStringLiteral("/a"));
        QCOMPARE(router.retainedLruWeight(), 2);

        QVERIFY(router.back().accepted);
        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/a"));
        QCOMPARE(provider->stats->created, 4);
    }

    void structuredProviderErrorIsPreservedInRouteFailure()
    {
        PageRegistry pages;
        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/broken");
        descriptor.provider = QStringLiteral("structured-failure");
        descriptor.fallbackPolicy = FallbackPolicy::None;
        QVERIFY(pages.registerPage(descriptor));

        PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(std::make_shared<StructuredFailureProvider>()));

        PageRouter router(&pages, &providers);
        const RouteResult result = router.push({QStringLiteral("/broken"), {}, {}});
        QVERIFY(!result.accepted);

        const QVariantMap details = result.error.details();
        QCOMPARE(details.value(QStringLiteral("primaryError")).toString(),
                 QStringLiteral("structured create failure"));
        const QVariantMap failure = details.value(QStringLiteral("primaryFailure")).toMap();
        QCOMPARE(failure.value(QStringLiteral("code")).toString(),
                 QStringLiteral("test.create-failed"));
        QCOMPARE(failure.value(QStringLiteral("message")).toString(),
                 QStringLiteral("structured create failure"));
        QCOMPARE(failure.value(QStringLiteral("details")).toMap()
                     .value(QStringLiteral("source")).toString(),
                 QStringLiteral("unit-test"));
    }
};

QTEST_APPLESS_MAIN(RuntimeCacheTest)
#include "tst_runtime_cache.moc"
