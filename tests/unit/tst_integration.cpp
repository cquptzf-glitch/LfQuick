#include <LfQuick/Integration/LfScannerTarget.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/MockScanner.h>
#include <LfQuick/Ui/LfAttached.h>

#include <QQuickItem>
#include <QSignalSpy>
#include <QTest>
#include <QtQml/qqml.h>

using namespace LfQuick;

class IntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void scannerTargetConsumesAttachedAcceptScannerPolicy()
    {
        InputRouter router;
        MockScanner scanner;
        scanner.session()->setMinimumLength(1);
        QVERIFY(router.registerSource(&scanner));

        QQuickItem target;
        target.setObjectName(QStringLiteral("sample-input"));
        auto *attached = qmlAttachedPropertiesObject<Lf>(&target, true);
        QVERIFY(attached);
        attached->setAcceptScanner(true);

        LfScannerTarget scannerTarget;
        scannerTarget.setFollowActiveFocus(false);
        scannerTarget.setActive(true);
        scannerTarget.setRouter(&router);
        scannerTarget.setTarget(&target);

        QCOMPARE(router.targetCount(), 1);
        QCOMPARE(scannerTarget.targetId(), QString{});
        QVERIFY(scannerTarget.acceptScanner());
        QVERIFY(scannerTarget.active());
        QCOMPARE(scannerTarget.inputTarget()->id(), QStringLiteral("sample-input"));

        QSignalSpy received(&scannerTarget, &LfScannerTarget::inputReceived);
        scanner.send(QStringLiteral("S001"));
        QCOMPARE(received.count(), 1);
        QCOMPARE(received.at(0).at(0).toString(), QStringLiteral("S001"));

        attached->setAcceptScanner(false);
        QVERIFY(!scannerTarget.acceptScanner());
        scanner.send(QStringLiteral("S002"));
        QCOMPARE(received.count(), 1);
    }

    void scannerTargetClearsDestroyedRouter()
    {
        LfScannerTarget scannerTarget;
        auto *router = new InputRouter;
        scannerTarget.setRouter(router);
        QCOMPARE(scannerTarget.router(), router);

        QSignalSpy changed(&scannerTarget, &LfScannerTarget::routerChanged);
        delete router;

        QCOMPARE(scannerTarget.router(), nullptr);
        QCOMPARE(changed.count(), 1);
    }

    void scannerTargetClearsDestroyedTarget()
    {
        InputRouter router;
        LfScannerTarget scannerTarget;
        scannerTarget.setRouter(&router);

        auto *target = new QQuickItem;
        auto *attached = qmlAttachedPropertiesObject<Lf>(target, true);
        QVERIFY(attached);
        attached->setAcceptScanner(true);
        scannerTarget.setTarget(target);
        QCOMPARE(scannerTarget.target(), target);

        QSignalSpy changed(&scannerTarget, &LfScannerTarget::targetChanged);
        delete target;

        QCOMPARE(scannerTarget.target(), nullptr);
        QCOMPARE(scannerTarget.inputTarget()->target(), nullptr);
        QCOMPARE(changed.count(), 1);
    }
};

QTEST_MAIN(IntegrationTest)
#include "tst_integration.moc"
