#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <QTest>

#include <memory>

using namespace LfQuick;

namespace {

struct Counters {
    int created = 0;
    int activated = 0;
    int destroyed = 0;
};

class StressPage final : public IPageInstance
{
public:
    StressPage(QString route, std::shared_ptr<Counters> counters)
        : m_route(std::move(route)), m_counters(std::move(counters)) {}

    QString pageId() const override { return m_route; }
    void activate() override { ++m_counters->activated; }
    void deactivate() override {}
    void suspend() override {}
    void resume() override {}
    void destroy() override { ++m_counters->destroyed; }

private:
    QString m_route;
    std::shared_ptr<Counters> m_counters;
};

class StressProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("stress"); }

    bool canCreate(const PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }

    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &) override
    {
        ++counters->created;
        return std::make_unique<StressPage>(descriptor.route, counters);
    }

    std::shared_ptr<Counters> counters = std::make_shared<Counters>();
};

PageDescriptor stressPage(const QString &route)
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("stress");
    descriptor.cachePolicy = PageCachePolicy::None;
    descriptor.fallbackPolicy = FallbackPolicy::None;
    return descriptor;
}

} // namespace

class RuntimeStressTest : public QObject
{
    Q_OBJECT

private slots:
    void pushBackRecreatesPagesForOneThousandCycles()
    {
        PageRegistry pages;
        QVERIFY(pages.registerPage(stressPage(QStringLiteral("/a"))));
        QVERIFY(pages.registerPage(stressPage(QStringLiteral("/b"))));

        auto provider = std::make_shared<StressProvider>();
        PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        PageRouter router(&pages, &providers);
        QVERIFY(router.push({QStringLiteral("/a"), {}, {}}).accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/a"));
        QCOMPARE(router.depth(), 1);

        constexpr int cycles = 1000;
        for (int cycle = 0; cycle < cycles; ++cycle) {
            const QVariantMap parameters{{QStringLiteral("cycle"), cycle}};
            QVERIFY2(router.push({QStringLiteral("/b"), parameters, {}}).accepted,
                     qPrintable(QStringLiteral("push failed at cycle %1").arg(cycle)));
            QCOMPARE(router.currentRoute(), QStringLiteral("/b"));
            QCOMPARE(router.depth(), 2);

            QVERIFY2(router.back().accepted,
                     qPrintable(QStringLiteral("back failed at cycle %1").arg(cycle)));
            QCOMPARE(router.currentRoute(), QStringLiteral("/a"));
            QCOMPARE(router.depth(), 1);
        }

        QCOMPARE(provider->counters->created, 1 + cycles * 2);
        QCOMPARE(provider->counters->activated, 1 + cycles * 2);
        QCOMPARE(provider->counters->destroyed, cycles * 2);
        QCOMPARE(router.history(), QStringList{QStringLiteral("/a")});
        QVERIFY(!router.canGoBack());
    }
};

QTEST_MAIN(RuntimeStressTest)
#include "tst_runtime_stress.moc"
