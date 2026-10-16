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
        return std::make_unique<FakePage>(descriptor.route, counters);
    }

    std::shared_ptr<Counters> counters = std::make_shared<Counters>();
};

} // namespace

class RuntimeTests : public QObject
{
    Q_OBJECT
private slots:
    void registersPage()
    {
        LfQuick::PageRegistry registry;
        LfQuick::PageDescriptor page;
        page.route = "/dashboard";
        page.provider = "quick";
        QVERIFY(registry.registerPage(page));
        QVERIFY(!registry.registerPage(page));
        QCOMPARE(registry.size(), 1);
        QVERIFY(registry.find("/dashboard").has_value());
    }

    void lifecycleRejectsInvalidTransition()
    {
        LfQuick::PageLifecycle lifecycle;
        QVERIFY(!lifecycle.transitionTo(LfQuick::PageLifecycle::State::Active));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Ready));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Activating));
        QVERIFY(lifecycle.transitionTo(LfQuick::PageLifecycle::State::Active));
    }

    void routerPushAndBack()
    {
        LfQuick::PageRegistry pages;
        LfQuick::PageDescriptor dashboard;
        dashboard.route = "/dashboard";
        dashboard.provider = "test";
        QVERIFY(pages.registerPage(dashboard));

        LfQuick::PageDescriptor samples;
        samples.route = "/samples";
        samples.provider = "test";
        QVERIFY(pages.registerPage(samples));

        auto provider = std::make_shared<FakeProvider>();
        LfQuick::PageProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::PageRouter router(&pages, &providers);
        QVERIFY(router.push({QStringLiteral("/dashboard"), {}, {}}).accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/dashboard"));
        QVERIFY(router.push({QStringLiteral("/samples"), {}, {}}).accepted);
        QCOMPARE(router.depth(), 2);
        QVERIFY(router.canGoBack());
        QVERIFY(router.back().accepted);
        QCOMPARE(router.currentRoute(), QStringLiteral("/dashboard"));
        QVERIFY(provider->counters->activated >= 3);
        QVERIFY(provider->counters->destroyed >= 1);
    }
};

QTEST_APPLESS_MAIN(RuntimeTests)
#include "tst_runtime.moc"
