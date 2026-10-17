#include <LfQuick/Tooling/HotReloadStateTransfer.h>
#include <LfQuick/Tooling/QmlHotReloadController.h>
#include <LfQuick/Tooling/QmlHotReloadHost.h>
#include <LfQuick/Ui/LfAttached.h>

#include <QFile>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <QtQml/qqml.h>

using namespace LfQuick;

namespace {

class TestHotReloadHost : public QmlHotReloadHost
{
public:
    using QmlHotReloadHost::componentComplete;
};

class TestStateObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)

public:
    using QObject::QObject;

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

LfAttached *setReloadMetadata(QObject *object,
                              const QString &key,
                              const QStringList &properties = {})
{
    auto *attached = qmlAttachedPropertiesObject<Lf>(object, true);
    attached->setReloadKey(key);
    attached->setStateProperties(properties);
    return attached;
}

bool writeTextFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return file.write(content) == content.size();
}

} // namespace

class ToolingTests : public QObject
{
    Q_OBJECT

private slots:
    void controllerMapsLocalDevelopmentSourceAndInvalidatesCache()
    {
        QQmlEngine engine;
        QmlHotReloadController controller;
        controller.setEngine(&engine);
        controller.setDebounceMs(0);

        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("Preview.qml"));
        QVERIFY(writeTextFile(path, "import QtQuick\nItem {}\n"));

        const QUrl logical(QStringLiteral("qrc:/demo/Preview.qml"));
        const QUrl development = QUrl::fromLocalFile(path);
        QVERIFY(controller.watchSource(logical, development));
        QCOMPARE(controller.watchedSourceCount(), 1);
        QCOMPARE(controller.resolveSource(logical), development);

        QSignalSpy reloadSpy(&controller, &QmlHotReloadController::reloadRequested);
        controller.requestReload(logical);
        QCOMPARE(controller.revision(), 1);
        QCOMPARE(reloadSpy.count(), 1);
        const QList<QVariant> arguments = reloadSpy.takeFirst();
        QCOMPARE(arguments.at(0).toStringList(), QStringList{logical.toString(QUrl::FullyEncoded)});
        QCOMPARE(arguments.at(1).toLongLong(), 1);

        QVERIFY(controller.unwatchSource(logical));
        QCOMPARE(controller.watchedSourceCount(), 0);
        QCOMPARE(controller.resolveSource(logical), logical);
    }

    void sharedWatchOwnershipIsReferenceCounted()
    {
        QmlHotReloadController controller;
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        const QString path = temporary.filePath(QStringLiteral("Shared.qml"));
        const QString otherPath = temporary.filePath(QStringLiteral("Other.qml"));
        QVERIFY(writeTextFile(path, "import QtQuick\nItem {}\n"));
        QVERIFY(writeTextFile(otherPath, "import QtQuick\nItem {}\n"));

        const QUrl logical(QStringLiteral("qrc:/demo/Shared.qml"));
        const QUrl development = QUrl::fromLocalFile(path);
        QVERIFY(controller.watchSource(logical, development));
        QVERIFY(controller.watchSource(logical, development));
        QCOMPARE(controller.watchedSourceCount(), 1);

        QVERIFY(!controller.watchSource(logical, QUrl::fromLocalFile(otherPath)));
        QVERIFY(!controller.lastError().isEmpty());
        QCOMPARE(controller.resolveSource(logical), development);

        QVERIFY(controller.unwatchSource(logical));
        QCOMPARE(controller.watchedSourceCount(), 1);
        QCOMPARE(controller.resolveSource(logical), development);

        QVERIFY(controller.unwatchSource(logical));
        QCOMPARE(controller.watchedSourceCount(), 0);
        QCOMPARE(controller.resolveSource(logical), logical);
        QVERIFY(!controller.unwatchSource(logical));
    }

    void hostsReleaseOnlyTheirOwnSharedWatch()
    {
        QmlHotReloadController controller;
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("SharedHost.qml"));
        QVERIFY(writeTextFile(path, "import QtQuick\nItem {}\n"));

        const QUrl logical(QStringLiteral("qrc:/demo/SharedHost.qml"));
        const QUrl development = QUrl::fromLocalFile(path);

        {
            TestHotReloadHost first;
            first.setSource(logical);
            first.setDevelopmentSource(development);
            first.setController(&controller);
            QCOMPARE(controller.watchedSourceCount(), 1);

            {
                TestHotReloadHost second;
                second.setSource(logical);
                second.setDevelopmentSource(development);
                second.setController(&controller);
                QCOMPARE(controller.watchedSourceCount(), 1);
            }

            QCOMPARE(controller.watchedSourceCount(), 1);
            QCOMPARE(controller.resolveSource(logical), development);
        }

        QCOMPARE(controller.watchedSourceCount(), 0);
        QCOMPARE(controller.resolveSource(logical), logical);
    }

    void stateTransferUsesScopedStableKeys()
    {
        QObject oldRoot;
        QObject newRoot;

        QObject oldLeft(&oldRoot);
        QObject oldRight(&oldRoot);
        setReloadMetadata(&oldLeft, QStringLiteral("left"));
        setReloadMetadata(&oldRight, QStringLiteral("right"));

        TestStateObject oldLeftValue(&oldLeft);
        TestStateObject oldRightValue(&oldRight);
        setReloadMetadata(&oldLeftValue, QStringLiteral("value"), {QStringLiteral("counter")});
        setReloadMetadata(&oldRightValue, QStringLiteral("value"), {QStringLiteral("counter")});
        oldLeftValue.setCounter(41);
        oldRightValue.setCounter(82);

        QObject newLeft(&newRoot);
        QObject newRight(&newRoot);
        setReloadMetadata(&newLeft, QStringLiteral("left"));
        setReloadMetadata(&newRight, QStringLiteral("right"));

        TestStateObject newLeftValue(&newLeft);
        TestStateObject newRightValue(&newRight);
        setReloadMetadata(&newLeftValue, QStringLiteral("value"), {QStringLiteral("counter")});
        setReloadMetadata(&newRightValue, QStringLiteral("value"), {QStringLiteral("counter")});
        newLeftValue.setCounter(1);
        newRightValue.setCounter(2);

        const HotReloadTransferStats stats = HotReloadStateTransfer::transfer(&newRoot, &oldRoot);
        QCOMPARE(newLeftValue.counter(), 41);
        QCOMPARE(newRightValue.counter(), 82);
        QCOMPARE(stats.matchedObjects, 4);
        QCOMPARE(stats.restoredProperties, 2);
        QCOMPARE(stats.skippedProperties, 0);
        QVERIFY(stats.ambiguousKeys.isEmpty());
    }

    void stateTransferRejectsAmbiguousKeysWithinScope()
    {
        QObject oldRoot;
        QObject newRoot;

        TestStateObject oldFirst(&oldRoot);
        TestStateObject oldSecond(&oldRoot);
        setReloadMetadata(&oldFirst, QStringLiteral("duplicate"), {QStringLiteral("counter")});
        setReloadMetadata(&oldSecond, QStringLiteral("duplicate"), {QStringLiteral("counter")});
        oldFirst.setCounter(11);
        oldSecond.setCounter(22);

        TestStateObject replacement(&newRoot);
        setReloadMetadata(&replacement, QStringLiteral("duplicate"), {QStringLiteral("counter")});
        replacement.setCounter(7);

        const HotReloadTransferStats stats = HotReloadStateTransfer::transfer(&newRoot, &oldRoot);
        QCOMPARE(replacement.counter(), 7);
        QCOMPARE(stats.matchedObjects, 0);
        QCOMPARE(stats.restoredProperties, 0);
        QCOMPARE(stats.ambiguousKeys, QStringList{QStringLiteral("duplicate")});
    }

    void hostSwapsOnlyAfterSuccessfulCreationAndPreservesSelectedState()
    {
        QQmlEngine engine;
        QmlHotReloadController controller;
        controller.setEngine(&engine);
        controller.setDebounceMs(0);

        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("Preview.qml"));
        QVERIFY(writeTextFile(
            path,
            "import QtQuick\nItem { property int counter: 7; property string version: 'one' }\n"));

        const QUrl logical(QStringLiteral("qrc:/demo/Preview.qml"));
        TestHotReloadHost host;
        host.setSource(logical);
        host.setDevelopmentSource(QUrl::fromLocalFile(path));
        host.setPreservedProperties({QStringLiteral("counter")});
        host.setController(&controller);
        host.componentComplete();

        QCOMPARE(host.status(), QmlHotReloadHost::Status::Ready);
        QVERIFY(host.item());
        QCOMPARE(host.item()->property("counter").toInt(), 7);
        QCOMPARE(host.item()->property("version").toString(), QStringLiteral("one"));

        QObject *firstItem = host.item();
        QVERIFY(firstItem->setProperty("counter", 42));
        QVERIFY(writeTextFile(
            path,
            "import QtQuick\nItem { property int counter: 1; property string version: 'two' }\n"));

        QSignalSpy successSpy(&host, &QmlHotReloadHost::reloadSucceeded);
        controller.requestReload(logical);

        QCOMPARE(successSpy.count(), 1);
        QCOMPARE(host.status(), QmlHotReloadHost::Status::Ready);
        QVERIFY(host.item());
        QVERIFY(host.item() != firstItem);
        QCOMPARE(host.item()->property("counter").toInt(), 42);
        QCOMPARE(host.item()->property("version").toString(), QStringLiteral("two"));
        QCOMPARE(host.revision(), 1);

        QObject *stableItem = host.item();
        QVERIFY(writeTextFile(path, "import QtQuick\nItem { this is invalid qml }\n"));
        QSignalSpy failureSpy(&host, &QmlHotReloadHost::reloadFailed);
        controller.requestReload(logical);

        QCOMPARE(failureSpy.count(), 1);
        QCOMPARE(host.status(), QmlHotReloadHost::Status::Failed);
        QCOMPARE(host.item(), stableItem);
        QCOMPARE(host.item()->property("counter").toInt(), 42);
        QCOMPARE(host.item()->property("version").toString(), QStringLiteral("two"));
        QCOMPARE(host.revision(), 1);
        QVERIFY(!host.lastError().isEmpty());
    }
};

QTEST_MAIN(ToolingTests)
#include "tst_tooling.moc"
