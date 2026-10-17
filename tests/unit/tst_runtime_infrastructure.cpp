#include <LfQuick/Tooling/HotReloadStateTransfer.h>
#include <LfQuick/Ui/LifetimeLease.h>
#include <LfQuick/Ui/PersistentState.h>
#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QFile>
#include <QQmlEngine>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
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
