#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/WindowRouter.h>

#include <QtTest>

#include <memory>

namespace {

struct State {
    int created = 0;
    int closed = 0;
    QStringList closeOrder;
};

class OwnerAwareWindow final : public LfQuick::IWindowInstance
{
public:
    OwnerAwareWindow(QString id, std::shared_ptr<State> state)
        : m_id(std::move(id)), m_state(std::move(state)) {}

    QString instanceId() const override { return m_id; }
    void activate() override {}
    void deactivate() override {}
    bool canUpdateOwner(const QString &ownerInstanceId) const override
    {
        return acceptOwnerUpdates && ownerInstanceId != m_id;
    }
    bool updateOwner(const QString &ownerInstanceId) override
    {
        if (!canUpdateOwner(ownerInstanceId))
            return false;
        owner = ownerInstanceId;
        ++ownerUpdates;
        return true;
    }
    bool updateParameters(const QVariantMap &) override { return true; }
    QVariantMap saveState() const override { return savedState; }
    void close() override
    {
        ++m_state->closed;
        m_state->closeOrder.append(m_id);
    }

    QString owner;
    int ownerUpdates = 0;
    bool acceptOwnerUpdates = true;
    QVariantMap savedState;

private:
    QString m_id;
    std::shared_ptr<State> m_state;
};

class OwnerAwareProvider final : public LfQuick::IWindowProvider
{
public:
    QString id() const override { return QStringLiteral("owner-aware"); }
    bool canCreate(const LfQuick::WindowDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }

    std::unique_ptr<LfQuick::IWindowInstance> create(
        const LfQuick::WindowDescriptor &,
        const LfQuick::WindowOpenRequest &request) override
    {
        auto window = std::make_unique<OwnerAwareWindow>(
            QStringLiteral("owner-window-%1").arg(++state->created), state);
        if (!request.ownerInstanceId.isEmpty())
            window->updateOwner(request.ownerInstanceId);
        lastCreated = window.get();
        windows.push_back(window.get());
        return window;
    }

    std::shared_ptr<State> state = std::make_shared<State>();
    OwnerAwareWindow *lastCreated = nullptr;
    QList<OwnerAwareWindow *> windows;
};

LfQuick::WindowDescriptor descriptor(
    QString id,
    LfQuick::WindowOpenPolicy openPolicy = LfQuick::WindowOpenPolicy::SingleInstance,
    LfQuick::OwnerClosePolicy ownerPolicy = LfQuick::OwnerClosePolicy::CloseWithOwner)
{
    LfQuick::WindowDescriptor result;
    result.id = std::move(id);
    result.provider = QStringLiteral("owner-aware");
    result.openPolicy = openPolicy;
    result.ownerClosePolicy = ownerPolicy;
    return result;
}

} // namespace

class WindowOwnerTests : public QObject
{
    Q_OBJECT

private slots:
    void explicitReparentUpdatesAdapterAndRuntime()
    {
        auto provider = std::make_shared<OwnerAwareProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("owner-a"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("owner-b"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("tool"),
                                                  LfQuick::WindowOpenPolicy::MultipleInstances,
                                                  LfQuick::OwnerClosePolicy::Detach)));

        const auto ownerA = router.open({QStringLiteral("owner-a"), {}, {}, {}, {}});
        const auto ownerB = router.open({QStringLiteral("owner-b"), {}, {}, {}, {}});
        const auto tool = router.open({QStringLiteral("tool"), {}, ownerA.instanceId, {}, {}});
        QVERIFY(ownerA.accepted && ownerB.accepted && tool.accepted);

        auto *toolWindow = provider->lastCreated;
        QCOMPARE(toolWindow->owner, ownerA.instanceId);
        QVERIFY(router.reparent(tool.instanceId, ownerB.instanceId).accepted);
        QCOMPARE(toolWindow->owner, ownerB.instanceId);
        QCOMPARE(router.find(tool.instanceId)->ownerInstanceId, ownerB.instanceId);

        QVERIFY(router.reparent(tool.instanceId, {}).accepted);
        QVERIFY(toolWindow->owner.isEmpty());
        QVERIFY(router.find(tool.instanceId)->ownerInstanceId.isEmpty());
    }

    void singleInstanceReuseRebindsOwner()
    {
        auto provider = std::make_shared<OwnerAwareProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("owner-a"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("owner-b"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("tool"))));

        const auto ownerA = router.open({QStringLiteral("owner-a"), {}, {}, {}, {}});
        const auto ownerB = router.open({QStringLiteral("owner-b"), {}, {}, {}, {}});
        const auto first = router.open({QStringLiteral("tool"), {}, ownerA.instanceId, {}, {}});
        QVERIFY(first.accepted);
        auto *toolWindow = provider->lastCreated;

        const auto reused = router.open({QStringLiteral("tool"), {}, ownerB.instanceId, {}, {}});
        QVERIFY(reused.accepted);
        QCOMPARE(reused.instanceId, first.instanceId);
        QCOMPARE(toolWindow->owner, ownerB.instanceId);
        QCOMPARE(router.find(first.instanceId)->ownerInstanceId, ownerB.instanceId);
    }

    void nestedDetachSurvivorIsUnownedBeforeAncestorsClose()
    {
        auto provider = std::make_shared<OwnerAwareProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("root"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("middle"),
                                                  LfQuick::WindowOpenPolicy::MultipleInstances,
                                                  LfQuick::OwnerClosePolicy::CloseWithOwner)));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("leaf"),
                                                  LfQuick::WindowOpenPolicy::MultipleInstances,
                                                  LfQuick::OwnerClosePolicy::Detach)));

        const auto root = router.open({QStringLiteral("root"), {}, {}, {}, {}});
        const auto middle = router.open({QStringLiteral("middle"), {}, root.instanceId, {}, {}});
        const auto leaf = router.open({QStringLiteral("leaf"), {}, middle.instanceId, {}, {}});
        QVERIFY(root.accepted && middle.accepted && leaf.accepted);
        auto *leafWindow = provider->lastCreated;
        QCOMPARE(leafWindow->owner, middle.instanceId);

        QVERIFY(router.close(root.instanceId).accepted);
        QCOMPARE(router.count(), 1);
        QCOMPARE(provider->state->closed, 2);
        QVERIFY(router.find(leaf.instanceId).has_value());
        QVERIFY(router.find(leaf.instanceId)->ownerInstanceId.isEmpty());
        QVERIFY(leafWindow->owner.isEmpty());
    }

    void closeAllClosesChildrenBeforeOwnersAndRemovesBeforeSignal()
    {
        auto provider = std::make_shared<OwnerAwareProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("root"))));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("child"),
                                                  LfQuick::WindowOpenPolicy::MultipleInstances)));
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("leaf"),
                                                  LfQuick::WindowOpenPolicy::MultipleInstances)));

        const auto root = router.open({QStringLiteral("root"), {}, {}, {}, {}});
        const auto child = router.open({QStringLiteral("child"), {}, root.instanceId, {}, {}});
        const auto leaf = router.open({QStringLiteral("leaf"), {}, child.instanceId, {}, {}});
        QVERIFY(root.accepted && child.accepted && leaf.accepted);

        QList<int> countsDuringClose;
        connect(&router, &LfQuick::WindowRouter::windowClosed,
                &router, [&router, &countsDuringClose](const QString &, const QString &) {
            countsDuringClose.append(router.count());
        });

        QVERIFY(router.closeAll().accepted);
        QCOMPARE(provider->state->closeOrder,
                 QStringList({leaf.instanceId, child.instanceId, root.instanceId}));
        QCOMPARE(countsDuringClose, QList<int>({2, 1, 0}));
        QCOMPARE(router.count(), 0);
    }

    void saveStateUsesProviderNativeState()
    {
        auto provider = std::make_shared<OwnerAwareProvider>();
        LfQuick::WindowProviderRegistry providers;
        QVERIFY(providers.registerProvider(provider));

        LfQuick::WindowRouter router(&providers);
        QVERIFY(router.registerWindow(descriptor(QStringLiteral("stateful"))));
        const auto opened = router.open({QStringLiteral("stateful"), {}, {}, {}, {}});
        QVERIFY(opened.accepted);

        provider->lastCreated->savedState = {
            {QStringLiteral("geometry"), QStringLiteral("saved-geometry")},
            {QStringLiteral("maximized"), true}
        };

        const auto state = router.saveState(opened.instanceId);
        QVERIFY(state.has_value());
        QCOMPARE(state->value(QStringLiteral("geometry")).toString(), QStringLiteral("saved-geometry"));
        QVERIFY(state->value(QStringLiteral("maximized")).toBool());
        QVERIFY(!router.saveState(QStringLiteral("missing-window")).has_value());
    }
};

QTEST_MAIN(WindowOwnerTests)
#include "tst_window_owner.moc"
