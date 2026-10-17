#include <LfQuick/Tooling/HotReloadStateTransfer.h>
#include <LfQuick/Ui/PersistentState.h>
#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QFile>
#include <QQmlEngine>
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

bool writeQml(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const QByteArray data =
        "import QtQml\n"
        "QtObject { property int answer: 1 }\n";
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
};

QTEST_MAIN(RuntimeInfrastructureTest)
#include "tst_runtime_infrastructure.moc"
