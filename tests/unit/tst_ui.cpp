#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/QuickRuntime.h>

#include <LfQuick/Core/OperationHandle.h>

#include <QTest>
#include <QQmlEngine>
#include <QUrl>

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

    void quickRuntimeRejectsUnknownRoute()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);

        QVERIFY(runtime.registerPage(QStringLiteral("/missing-source"),
                                     QUrl(QStringLiteral("file:///definitely/not/a/page.qml"))));
        QVERIFY(!runtime.push(QStringLiteral("/unknown")));
        QVERIFY(!runtime.push(QStringLiteral("/missing-source")));
        QCOMPARE(runtime.currentRoute(), QString{});
    }
};
QTEST_MAIN(UiTest)
#include "tst_ui.moc"
