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
};

QTEST_MAIN(QuickPageLifecycleTest)
#include "tst_quick_page_lifecycle.moc"
