#include <LfQuick/Tooling/HotReloadStateTransfer.h>
#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/LifetimeLease.h>
#include <LfQuick/Ui/PersistentState.h>
#include <LfQuick/Ui/QmlIncubationScheduler.h>
#include <LfQuick/Ui/QmlObjectFactory.h>
#include <LfQuick/Ui/QmlStateSnapshot.h>

#include <QFile>
#include <QQmlEngine>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtQml/qqml.h>
#include <QtTest>

using namespace LfQuick;

namespace {

class TestPersistentState final : public PersistentState
{
    Q_OBJECT
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)

public:
    using PersistentState::PersistentState;

    int counter() const noexcept { return m_counter; }
    void setCounter(int value)
    {
        if (m_counter == value)
            return;
        m_counter = value;
        emit counterChanged();
    }

signals:
    void counterChanged();

private:
    int m_counter = 0;
};

LfAttached *reloadMetadata(QObject *object, const QString &key)
{
    auto *attached = qobject_cast<LfAttached *>(
        qmlAttachedPropertiesObject<Lf>(object, true));
    if (attached)
        attached->setReloadKey(key);
    return attached;
}

bool writeQml(const QString &path, bool required = false)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const QByteArray data = required
        ? QByteArray("import QtQml\nQtObject { required property int answer }\n")
        : QByteArray("import QtQml\nQtObject { property int answer: 1 }\n");
    return file.write(data) == data.size();
}

} // namespace

class RuntimeInfrastructureTest : public QObject
{
    Q_OBJECT

private slots:
    void persistentStateRestoresQmlDeclaredEquivalentProperties()
    {
        TestPersistentState oldState;
        oldState.setKey(QStringLiteral("filters"));
        oldState.setCounter(55);

        TestPersistentState newState;
        newState.setKey(QStringLiteral("filters"));
        newState.setCounter(1);

        QSignalSpy loaded(&newState, &PersistentState::loaded);
        QSignalSpy reloaded(&newState, &PersistentState::reloaded);

        const HotReloadTransferStats stats =
            HotReloadStateTransfer::transfer(&newState, &oldState);

        QCOMPARE(newState.counter(), 55);
        QCOMPARE(stats.matchedObjects, 1);
        QCOMPARE(stats.restoredProperties, 1);
        QCOMPARE(stats.skippedProperties, 0);
        QCOMPARE(loaded.count(), 1);
        QCOMPARE(reloaded.count(), 1);
    }

    void duplicateReloadScopeInvalidatesEntireDescendantSubtree()
    {
        QObject oldRoot;
        auto *oldPanelA = new QObject(&oldRoot);
        auto *oldPanelB = new QObject(&oldRoot);
        QVERIFY(reloadMetadata(oldPanelA, QStringLiteral("panel")));
        QVERIFY(reloadMetadata(oldPanelB, QStringLiteral("panel")));

        auto *oldLeft = new TestPersistentState(oldPanelA);
        oldLeft->setKey(QStringLiteral("left"));
        oldLeft->setCounter(11);
        auto *oldRight = new TestPersistentState(oldPanelB);
        oldRight->setKey(QStringLiteral("right"));
        oldRight->setCounter(22);

        const QVariantMap snapshot = QmlStateSnapshot::capture(&oldRoot);
        const QStringList ambiguous =
            snapshot.value(QStringLiteral("ambiguousKeys")).toStringList();
        QVERIFY(ambiguous.contains(QStringLiteral("panel")));
        QVERIFY(ambiguous.contains(QStringLiteral("panel/left")));
        QVERIFY(ambiguous.contains(QStringLiteral("panel/right")));

        QObject newRoot;
        auto *newPanelA = new QObject(&newRoot);
        auto *newPanelB = new QObject(&newRoot);
        QVERIFY(reloadMetadata(newPanelA, QStringLiteral("panel")));
        QVERIFY(reloadMetadata(newPanelB, QStringLiteral("panel")));

        auto *newLeft = new TestPersistentState(newPanelA);
        newLeft->setKey(QStringLiteral("left"));
        newLeft->setCounter(1);
        auto *newRight = new TestPersistentState(newPanelB);
        newRight->setKey(QStringLiteral("right"));
        newRight->setCounter(2);

        QSignalSpy leftLoaded(newLeft, &PersistentState::loaded);
        QSignalSpy rightLoaded(newRight, &PersistentState::loaded);
        const QmlStateRestoreStats stats =
            QmlStateSnapshot::restore(&newRoot, snapshot);

        // A unique child key cannot repair an ambiguous parent identity.
        QCOMPARE(newLeft->counter(), 1);
        QCOMPARE(newRight->counter(), 2);
        QCOMPARE(stats.restoredProperties, 0);
        QVERIFY(stats.ambiguousKeys.contains(QStringLiteral("panel/left")));
        QVERIFY(stats.ambiguousKeys.contains(QStringLiteral("panel/right")));
        QCOMPARE(leftLoaded.count(), 1);
        QCOMPARE(rightLoaded.count(), 1);
    }

    void persistentStateStopsIfLoadedCallbackDestroysInstance()
    {
        QObject oldRoot;
        auto *oldState = new TestPersistentState(&oldRoot);
        oldState->setKey(QStringLiteral("state"));
        oldState->setCounter(7);

        QObject newRoot;
        auto *newState = new TestPersistentState(&newRoot);
        newState->setKey(QStringLiteral("state"));
        newState->setCounter(1);
        QPointer<TestPersistentState> guarded = newState;
        QSignalSpy reloaded(newState, &PersistentState::reloaded);

        connect(newState, &PersistentState::loaded, &newRoot, [newState] {
            delete newState;
        }, Qt::DirectConnection);

        const QmlStateRestoreStats stats =
            QmlStateSnapshot::transfer(&newRoot, &oldRoot);

        QVERIFY(guarded.isNull());
        QCOMPARE(stats.restoredProperties, 1);
        QCOMPARE(reloaded.count(), 0);
    }

    void lifetimeLeaseDetachesRetainedObjectFromQObjectOwner()
    {
        QObject owner;
        auto *target = new QObject(&owner);
        QPointer<QObject> guarded = target;

        LifetimeLease lease;
        lease.setObject(target);
        lease.setHeld(true);

        QVERIFY(!LifetimeLease::requestDestroy(target));
        QCOMPARE(target->parent(), nullptr);
        QVERIFY(LifetimeLease::isRetained(target));

        lease.setHeld(false);
        QTRY_VERIFY(guarded.isNull());
    }

    void incubationSchedulerReinstallsExistingHelper()
    {
        QQmlEngine engine;
        QmlIncubationScheduler *scheduler =
            QmlIncubationScheduler::ensureForEngine(&engine);
        QVERIFY(scheduler);
        QVERIFY(scheduler->isInstalled());
        QCOMPARE(engine.incubationController(), scheduler);

        scheduler->uninstall();
        QVERIFY(!scheduler->isInstalled());
        QCOMPARE(engine.incubationController(), nullptr);

        QmlIncubationScheduler *reused =
            QmlIncubationScheduler::ensureForEngine(&engine);
        QCOMPARE(reused, scheduler);
        QVERIFY(reused->isInstalled());
        QCOMPARE(engine.incubationController(), reused);
    }

    void objectFactoryCreatesSynchronouslyWithInitialProperties()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("FactoryObject.qml"));
        QVERIFY(writeQml(path));

        QQmlEngine engine;
        QmlObjectFactory factory;
        factory.setEngine(&engine);

        QPointer<QObject> object = factory.create(
            QUrl::fromLocalFile(path),
            {{QStringLiteral("answer"), 17}});
        QVERIFY2(object, qPrintable(factory.lastError()));
        QCOMPARE(object->property("answer").toInt(), 17);
        delete object;
    }

    void factoryNotifiesWhenExplicitEngineIsDestroyed()
    {
        QmlObjectFactory factory;
        auto *engine = new QQmlEngine;
        factory.setEngine(engine);
        QCOMPARE(factory.engine(), engine);

        QSignalSpy changed(&factory, &QmlObjectFactory::engineChanged);
        delete engine;

        QCOMPARE(factory.engine(), nullptr);
        QCOMPARE(changed.count(), 1);
    }

    void asyncObjectFactoryCanBeForcedToCompletion()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("AsyncFactoryObject.qml"));
        QVERIFY(writeQml(path));

        QQmlEngine engine;
        QmlObjectFactory factory;
        factory.setEngine(&engine);

        QmlObjectLoadHandle *handle = factory.createAsync(
            QUrl::fromLocalFile(path),
            {{QStringLiteral("answer"), 23}});
        QVERIFY(handle);

        handle->forceCompletion();
        QCOMPARE(handle->status(), QmlObjectLoadHandle::Status::Ready);
        QVERIFY2(handle->object(), qPrintable(handle->errorString()));

        QPointer<QObject> object = handle->takeObject();
        QVERIFY(object);
        QCOMPARE(object->property("answer").toInt(), 23);
        delete object;
    }

    void asyncObjectFactoryFinishesIfEngineDisappears()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("EngineLifetimeObject.qml"));
        QVERIFY(writeQml(path));

        QmlObjectFactory factory;
        auto *engine = new QQmlEngine;
        factory.setEngine(engine);

        QmlObjectLoadHandle *handle = factory.createAsync(
            QUrl::fromLocalFile(path),
            {{QStringLiteral("answer"), 29}});
        QVERIFY(handle);
        QCOMPARE(handle->status(), QmlObjectLoadHandle::Status::Loading);

        QSignalSpy finished(handle, &QmlObjectLoadHandle::finished);
        delete engine;

        QCOMPARE(factory.engine(), nullptr);
        QCOMPARE(handle->status(), QmlObjectLoadHandle::Status::Error);
        QCOMPARE(finished.count(), 1);
        QVERIFY(handle->errorString().contains(QStringLiteral("destroyed"), Qt::CaseInsensitive));
    }

    void asyncObjectFactoryHonorsRequiredInitialProperties()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("RequiredObject.qml"));
        QVERIFY(writeQml(path, true));

        QQmlEngine engine;
        QmlObjectFactory factory;
        factory.setEngine(&engine);

        QmlObjectLoadHandle *valid = factory.createAsync(
            QUrl::fromLocalFile(path),
            {{QStringLiteral("answer"), 31}});
        valid->forceCompletion();
        QCOMPARE(valid->status(), QmlObjectLoadHandle::Status::Ready);
        QPointer<QObject> validObject = valid->takeObject();
        QVERIFY(validObject);
        QCOMPARE(validObject->property("answer").toInt(), 31);
        delete validObject;

        QmlObjectLoadHandle *invalid = factory.createAsync(
            QUrl::fromLocalFile(path), {});
        invalid->forceCompletion();
        QCOMPARE(invalid->status(), QmlObjectLoadHandle::Status::Error);
        QVERIFY(!invalid->errorString().isEmpty());
    }
};

QTEST_MAIN(RuntimeInfrastructureTest)
#include "tst_runtime_infrastructure.moc"
