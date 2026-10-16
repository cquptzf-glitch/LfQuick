#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/WidgetBridge/WidgetPageProvider.h>

#include <QLabel>
#include <QStackedWidget>
#include <QTest>

using namespace LfQuick;

class WidgetBridgeTest : public QObject
{
    Q_OBJECT

private slots:
    void widgetProviderCreatesLegacyPageInSharedStack()
    {
        QStackedWidget host;
        WidgetPageProvider provider(&host);

        QVERIFY(provider.registerFactory(
            QStringLiteral("legacy-results"),
            [](QWidget *parent, const QVariantMap &parameters) -> QWidget * {
                auto *label = new QLabel(parameters.value(QStringLiteral("title")).toString(), parent);
                label->setObjectName(QStringLiteral("legacyResults"));
                return label;
            }));

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/legacy/results");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("legacy-results"));
        descriptor.cachePolicy = PageCachePolicy::KeepAlive;

        auto instance = provider.create(
            descriptor,
            {{QStringLiteral("title"), QStringLiteral("Legacy Results")}});

        QVERIFY(instance != nullptr);
        QCOMPARE(instance->pageId(), QStringLiteral("/legacy/results"));
        QCOMPARE(host.count(), 1);

        instance->activate();
        QCOMPARE(host.currentWidget()->objectName(), QStringLiteral("legacyResults"));
        QCOMPARE(qobject_cast<QLabel *>(host.currentWidget())->text(), QStringLiteral("Legacy Results"));

        instance->deactivate();
        instance->destroy();
    }

    void widgetProviderRejectsUnknownFactory()
    {
        QStackedWidget host;
        WidgetPageProvider provider(&host);

        PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/missing");
        descriptor.provider = QStringLiteral("widget");
        descriptor.source.insert(QStringLiteral("factory"), QStringLiteral("missing"));

        QVERIFY(!provider.canCreate(descriptor));
        QVERIFY(provider.create(descriptor, {}) == nullptr);
    }
};

QTEST_MAIN(WidgetBridgeTest)
#include "tst_widgetbridge.moc"
