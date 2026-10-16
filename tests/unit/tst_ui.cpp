#include <LfQuick/Ui/OverlayCoordinator.h>

#include <LfQuick/Core/OperationHandle.h>

#include <QTest>

using namespace LfQuick;

class UiTest : public QObject
{
    Q_OBJECT

private slots:
    void busyOverlayFollowsOperation()
    {
        OverlayCoordinator coordinator;
        OperationHandle operation;
        operation.setMessage(QStringLiteral("Calibrating"));
        operation.setCancelable(true);
        operation.setProgress(0.25);

        const QString token = coordinator.bindOperation(&operation);
        QVERIFY(!token.isEmpty());
        QVERIFY(coordinator.busy());
        QCOMPARE(coordinator.busyCount(), 1);
        QCOMPARE(coordinator.busyMessage(), QStringLiteral("Calibrating"));
        QCOMPARE(coordinator.busyProgress(), 0.25);
        QVERIFY(coordinator.busyCancelable());

        operation.setProgress(0.75);
        QCOMPARE(coordinator.busyProgress(), 0.75);

        operation.setState(OperationHandle::State::Succeeded);
        QVERIFY(!coordinator.busy());
    }
};
QTEST_MAIN(UiTest)
#include "tst_ui.moc"
