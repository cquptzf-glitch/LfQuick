#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
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

    void qmlErrorCollectorKeepsStructuredDiagnostics()
    {
        QmlErrorCollector collector;
        QSignalSpy recorded(&collector, &QmlErrorCollector::errorRecorded);

        const QUrl source(QStringLiteral("qrc:/BrokenPage.qml"));
        collector.record(QStringLiteral("/results"), source,
                         QStringLiteral("Type ResultPage unavailable"),
                         QStringLiteral("primary"));

        QCOMPARE(collector.count(), 1);
        QCOMPARE(recorded.count(), 1);
        const auto entry = collector.lastError();
        QCOMPARE(entry.value(QStringLiteral("route")).toString(), QStringLiteral("/results"));
        QCOMPARE(entry.value(QStringLiteral("source")).toUrl(), source);
        QCOMPARE(entry.value(QStringLiteral("stage")).toString(), QStringLiteral("primary"));
        QVERIFY(entry.value(QStringLiteral("message")).toString().contains(QStringLiteral("ResultPage")));

        collector.clear();
        QCOMPARE(collector.count(), 0);
    }

    void quickRuntimeRejectsUnknownRouteAndCanDisableFallback()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);

        QVERIFY(runtime.registerPage(
            QStringLiteral("/missing-source"),
            QUrl(QStringLiteral("file:///definitely/not/a/page.qml")),
            static_cast<int>(PageCachePolicy::KeepAlive),
            false));

        QVERIFY(!runtime.push(QStringLiteral("/unknown")));
        QVERIFY(!runtime.push(QStringLiteral("/missing-source")));
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.errorCollector()->count(), 1);
    }
};

QTEST_MAIN(UiTest)
#include "tst_ui.moc"
