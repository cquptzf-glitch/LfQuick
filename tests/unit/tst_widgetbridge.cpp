#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>
#include <LfQuick/WidgetBridge/WidgetPageProvider.h>

#include <QLabel>
#include <QLineEdit>
#include <QQuickItem>
#include <QQuickWidget>
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
        QVERIFY(provider.lastError().contains(QStringLiteral("missing")));
    }

    void focusBridgeTransfersWidgetTabIntoQuickBoundary()
    {
        QWidget window;
        QLineEdit before(&window);
        QQuickWidget surface(&window);
        QQuickItem entry;
        entry.setParentItem(surface.rootObject() ? qobject_cast<QQuickItem *>(surface.rootObject()) : nullptr);
        QLineEdit after(&window);

        before.show();
        surface.show();
        after.show();
        window.show();
        QTest::qWaitForWindowExposed(&window);

        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, &after);
        bridge.setQuickBoundaryItems(&entry, &entry);

        before.setFocus();
        QTest::keyClick(&before, Qt::Key_Tab);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(&surface));
    }

    void focusBridgeScannerIsolationSuppressesTransfer()
    {
        QWidget window;
        QLineEdit before(&window);
        QQuickWidget surface(&window);
        QQuickItem entry;
        QLineEdit after(&window);

        before.show();
        surface.show();
        after.show();
        window.show();
        QTest::qWaitForWindowExposed(&window);

        WidgetFocusBridge bridge(&surface);
        bridge.setBoundaryWidgets(&before, &after);
        bridge.setQuickBoundaryItems(&entry, &entry);
        bridge.setScannerIsolation(true);

        before.setFocus();
        QTest::keyClick(&before, Qt::Key_Tab);
        QVERIFY(QApplication::focusWidget() != &surface);
    }
};

QTEST_MAIN(WidgetBridgeTest)
#include "tst_widgetbridge.moc"
