#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Ui/QuickPageProvider.h>

#include <QFile>
#include <QQmlEngine>
#include <QQuickItem>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

using namespace LfQuick;

class QuickPageLifecycleTest : public QObject
{
    Q_OBJECT

private slots:
    void activationCallbackMayDestroyHost()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        const QString qmlPath = temporary.filePath(QStringLiteral("Page.qml"));
        QFile qmlFile(qmlPath);
        QVERIFY(qmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(qmlFile.write("import QtQuick\nItem {}\n"), qint64(23));
        qmlFile.close();

        QQmlEngine engine;
        auto *host = new QQuickItem;
        QuickPageProvider provider(&engine, host);
        provider.setActivationCallback([&host] {
            delete host;
            host = nullptr;
        });

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/reentrant");
        descriptor.provider = QStringLiteral("quick");
        descriptor.source.insert(QStringLiteral("url"), QUrl::fromLocalFile(qmlPath));

        auto instance = provider.create(descriptor, {});
        QVERIFY(instance);
        QVERIFY(instance->isValid());

        instance->activate();

        QCOMPARE(host, nullptr);
        QVERIFY(!instance->isValid());
    }

    void stalePrewarmFallsBackToSynchronousCreation()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        const QString qmlPath = temporary.filePath(QStringLiteral("SelfDestroyingPage.qml"));
        QFile qmlFile(qmlPath);
        QVERIFY(qmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        const QByteArray qml =
            "import QtQuick\n"
            "Item {\n"
            "    Timer {\n"
            "        interval: 1\n"
            "        running: true\n"
            "        repeat: false\n"
            "        onTriggered: parent.destroy()\n"
            "    }\n"
            "}\n";
        QCOMPARE(qmlFile.write(qml), qint64(qml.size()));
        qmlFile.close();

        QQmlEngine engine;
        QQuickItem host;
        QuickPageProvider provider(&engine, &host);

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/stale-prewarm");
        descriptor.provider = QStringLiteral("quick");
        descriptor.source.insert(QStringLiteral("url"), QUrl::fromLocalFile(qmlPath));

        QVERIFY2(provider.prewarm(descriptor, {}), qPrintable(provider.lastError()));

        // Let asynchronous incubation finish and the prewarmed object delete
        // itself. The prewarm handle remains a cache entry, but its object is
        // no longer usable.
        QTest::qWait(100);

        auto instance = provider.create(descriptor, {});
        QVERIFY2(instance, qPrintable(provider.lastError()));
        QVERIFY(instance->isValid());
    }
};

QTEST_MAIN(QuickPageLifecycleTest)
#include "tst_quick_page_lifecycle.moc"
