#include <LfQuick/Ui/BusyToken.h>
#include <LfQuick/Ui/OverlayCoordinator.h>

#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class BusyTokenTest : public QObject
{
    Q_OBJECT

private slots:
    void releaseEndsBusyAndBecomesInactive()
    {
        OverlayCoordinator coordinator;
        BusyToken *token = coordinator.acquireBusy(QStringLiteral("Loading"), true);
        QVERIFY(token);
        QVERIFY(token->active());
        QVERIFY(coordinator.busy());
        QCOMPARE(coordinator.busyCount(), 1);
        QCOMPARE(coordinator.busyMessage(), QStringLiteral("Loading"));

        QSignalSpy released(token, &BusyToken::released);
        QVERIFY(token->release());
        QVERIFY(!token->active());
        QVERIFY(!coordinator.busy());
        QCOMPARE(coordinator.busyCount(), 0);
        QCOMPARE(released.count(), 1);
        QVERIFY(!token->release());

        delete token;
    }

    void legacyEndBusySynchronizesTokenState()
    {
        OverlayCoordinator coordinator;
        BusyToken *token = coordinator.acquireBusy(QStringLiteral("Legacy compatible"));
        QVERIFY(token);
        const QString id = token->id();
        QVERIFY(coordinator.hasBusyToken(id));

        QSignalSpy released(token, &BusyToken::released);
        QVERIFY(coordinator.endBusy(id));
        QVERIFY(!coordinator.hasBusyToken(id));
        QVERIFY(!token->active());
        QCOMPARE(released.count(), 1);

        delete token;
    }

    void destructionAutomaticallyEndsBusy()
    {
        OverlayCoordinator coordinator;
        BusyToken *token = coordinator.acquireBusy(QStringLiteral("Scoped"));
        QVERIFY(token);
        const QString id = token->id();
        QVERIFY(coordinator.hasBusyToken(id));

        delete token;

        QVERIFY(!coordinator.hasBusyToken(id));
        QVERIFY(!coordinator.busy());
    }

    void coordinatorDestructionInvalidatesSurvivingToken()
    {
        auto *coordinator = new OverlayCoordinator;
        BusyToken *token = coordinator->acquireBusy(QStringLiteral("Owner lifetime"));
        QVERIFY(token);
        QVERIFY(token->active());

        QSignalSpy released(token, &BusyToken::released);
        delete coordinator;

        QVERIFY(!token->active());
        QCOMPARE(released.count(), 1);
        QVERIFY(!token->release());
        delete token;
    }
};

QTEST_APPLESS_MAIN(BusyTokenTest)
#include "tst_busy_token.moc"
