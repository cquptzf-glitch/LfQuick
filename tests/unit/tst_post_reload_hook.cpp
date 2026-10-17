#include <LfQuick/Ui/PersistentState.h>
#include <LfQuick/Ui/PostReloadHook.h>
#include <LfQuick/Ui/QmlStateSnapshot.h>

#include <QSignalSpy>
#include <QtTest>

using namespace LfQuick;

namespace {

class TestPersistentState final : public PersistentState
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

public:
    using PersistentState::PersistentState;

    int value() const noexcept { return m_value; }
    void setValue(int value)
    {
        if (m_value == value)
            return;
        m_value = value;
        emit valueChanged();
    }

signals:
    void valueChanged();

private:
    int m_value = 0;
};

class InspectingHook final : public PostReloadHook
{
    Q_OBJECT

public:
    explicit InspectingHook(TestPersistentState *state, QObject *parent = nullptr)
        : PostReloadHook(parent), m_state(state) {}

    int observedValue = -1;
    bool observedReload = false;
    int calls = 0;

protected:
    void onPostReload(bool reloaded) override
    {
        ++calls;
        observedReload = reloaded;
        observedValue = m_state ? m_state->value() : -1;
    }

private:
    TestPersistentState *m_state = nullptr;
};

} // namespace

class PostReloadHookTests : public QObject
{
    Q_OBJECT

private slots:
    void initialRestoreRunsLoadedAfterStatePhase()
    {
        QObject root;
        TestPersistentState state(&root);
        state.setKey(QStringLiteral("filters"));
        state.setValue(11);
        InspectingHook hook(&state, &root);

        QSignalSpy loaded(&hook, &PostReloadHook::loaded);
        QSignalSpy reloaded(&hook, &PostReloadHook::reloaded);
        QSignalSpy stateLoaded(&state, &PersistentState::loaded);

        const QmlStateRestoreStats stats = QmlStateSnapshot::restore(&root, {});
        QCOMPARE(stats.matchedObjects, 0);
        QCOMPARE(stateLoaded.count(), 1);
        QCOMPARE(loaded.count(), 1);
        QCOMPARE(reloaded.count(), 0);
        QCOMPARE(hook.calls, 1);
        QVERIFY(!hook.observedReload);
        QCOMPARE(hook.observedValue, 11);
        QVERIFY(hook.completed());
        QVERIFY(!hook.lastCompletionWasReload());
    }

    void reloadHookObservesFullyRestoredPersistentState()
    {
        QObject oldRoot;
        TestPersistentState oldState(&oldRoot);
        oldState.setKey(QStringLiteral("filters"));
        oldState.setValue(73);
        const QVariantMap snapshot = QmlStateSnapshot::capture(&oldRoot);

        QObject newRoot;
        TestPersistentState newState(&newRoot);
        newState.setKey(QStringLiteral("filters"));
        newState.setValue(1);
        InspectingHook hook(&newState, &newRoot);

        QSignalSpy stateReloaded(&newState, &PersistentState::reloaded);
        QSignalSpy hookReloaded(&hook, &PostReloadHook::reloaded);

        const QmlStateRestoreStats stats = QmlStateSnapshot::restore(&newRoot, snapshot);
        QCOMPARE(stats.matchedObjects, 1);
        QCOMPARE(stats.restoredProperties, 1);
        QCOMPARE(newState.value(), 73);
        QCOMPARE(stateReloaded.count(), 1);
        QCOMPARE(hookReloaded.count(), 1);
        QCOMPARE(hook.calls, 1);
        QVERIFY(hook.observedReload);
        QCOMPARE(hook.observedValue, 73);
        QVERIFY(hook.lastCompletionWasReload());
    }

    void transferWithoutOldRootIsInitialLoad()
    {
        QObject root;
        TestPersistentState state(&root);
        state.setKey(QStringLiteral("filters"));
        InspectingHook hook(&state, &root);

        QSignalSpy loaded(&hook, &PostReloadHook::loaded);
        QSignalSpy reloaded(&hook, &PostReloadHook::reloaded);

        QmlStateSnapshot::transfer(&root, nullptr);
        QCOMPARE(loaded.count(), 1);
        QCOMPARE(reloaded.count(), 0);
        QVERIFY(!hook.lastCompletionWasReload());
    }
};

QTEST_MAIN(PostReloadHookTests)
#include "tst_post_reload_hook.moc"
