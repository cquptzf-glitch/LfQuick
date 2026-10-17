#include <LfQuick/Tooling/QmlHotReloadController.h>
#include <LfQuick/Tooling/QmlHotReloadHost.h>

#include <QFile>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace LfQuick;

namespace {

class TestHotReloadHost : public QmlHotReloadHost
{
public:
    using QmlHotReloadHost::componentComplete;
};

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
