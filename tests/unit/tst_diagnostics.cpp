#include <LfQuick/Diagnostics/AccessibilityAudit.h>
#include <LfQuick/Diagnostics/DiagnosticsLogSink.h>
#include <LfQuick/Diagnostics/DiagnosticsModel.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickItem>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <thread>

using namespace LfQuick;

class DiagnosticsTests : public QObject
{
    Q_OBJECT

private slots:
    void boundedEventHistoryDropsOldest()
    {
        DiagnosticsModel model;
        model.setMaxEvents(2);

        model.recordEvent(QStringLiteral("runtime"), QStringLiteral("first"));
        model.recordEvent(QStringLiteral("runtime"), QStringLiteral("second"));
        model.recordEvent(QStringLiteral("runtime"), QStringLiteral("third"));

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.droppedCount(), 1);
        QCOMPARE(model.eventAt(0).value(QStringLiteral("name")).toString(), QStringLiteral("second"));
        QCOMPARE(model.eventAt(1).value(QStringLiteral("name")).toString(), QStringLiteral("third"));
        QCOMPARE(model.eventAt(0).value(QStringLiteral("sequence")).toLongLong(), 2);
        QCOMPARE(model.eventAt(1).value(QStringLiteral("sequence")).toLongLong(), 3);
    }

    void metricsTimingAndSnapshotAreGeneric()
    {
        DiagnosticsModel model;
        model.setMetric(QStringLiteral("runtime.route"), QStringLiteral("/dashboard"));
        QCOMPARE(model.incrementMetric(QStringLiteral("navigation.count")), 1);
        QCOMPARE(model.incrementMetric(QStringLiteral("navigation.count"), 2), 3);
        model.recordTiming(QStringLiteral("quickSurface.firstFrameMs"), 17.5,
                           {{QStringLiteral("surface"), QStringLiteral("main")}});

        QCOMPARE(model.metric(QStringLiteral("runtime.route")).toString(), QStringLiteral("/dashboard"));
        QCOMPARE(model.metric(QStringLiteral("navigation.count")).toLongLong(), 3);
        QCOMPARE(model.metric(QStringLiteral("quickSurface.firstFrameMs")).toDouble(), 17.5);
        QCOMPARE(model.count(), 1);

        const QVariantMap snapshot = model.snapshot();
        QCOMPARE(snapshot.value(QStringLiteral("schemaVersion")).toInt(), 1);
        QCOMPARE(snapshot.value(QStringLiteral("metrics")).toMap()
                     .value(QStringLiteral("navigation.count")).toLongLong(), 3);
        QCOMPARE(snapshot.value(QStringLiteral("events")).toList().size(), 1);
        QVERIFY(!snapshot.value(QStringLiteral("generatedAtUtc")).toString().isEmpty());

        QVERIFY(model.removeMetric(QStringLiteral("runtime.route")));
        QVERIFY(!model.removeMetric(QStringLiteral("runtime.route")));
    }

    void telemetrySinkRecordsSameThreadAndWorkerThreadEvents()
    {
        DiagnosticsModel model;
        DiagnosticsLogSink sink(&model);

        TelemetryEvent direct;
        direct.name = QStringLiteral("direct");
        direct.category = QStringLiteral("runtime");
        direct.level = TelemetryLevel::Warning;
        direct.durationMs = 12;
        direct.correlationId = QStringLiteral("corr-1");
        direct.attributes.insert(QStringLiteral("route"), QStringLiteral("/samples"));
        sink.write(direct);

        QCOMPARE(model.count(), 1);
        QVariantMap directEvent = model.eventAt(0);
        QCOMPARE(directEvent.value(QStringLiteral("name")).toString(), QStringLiteral("direct"));
        QCOMPARE(directEvent.value(QStringLiteral("severity")).toString(), QStringLiteral("warning"));
        QCOMPARE(directEvent.value(QStringLiteral("details")).toMap()
                     .value(QStringLiteral("durationMs")).toLongLong(), qint64(12));

        TelemetryEvent workerEvent;
        workerEvent.name = QStringLiteral("worker");
        workerEvent.category = QStringLiteral("background");
        workerEvent.attributes.insert(QStringLiteral("batch"), 7);

        std::thread worker([&sink, workerEvent] {
            sink.write(workerEvent);
        });
        worker.join();

        QTRY_COMPARE_WITH_TIMEOUT(model.count(), 2, 1000);
        QCOMPARE(model.eventAt(1).value(QStringLiteral("name")).toString(), QStringLiteral("worker"));
        QCOMPARE(model.eventAt(1).value(QStringLiteral("details")).toMap()
                     .value(QStringLiteral("batch")).toInt(), 7);
    }

    void accessibilityAuditClearsStateWhenRootDies()
    {
        AccessibilityAudit audit;
        auto *root = new QQuickItem;
        root->setObjectName(QStringLiteral("audit-root"));
        root->setFocusPolicy(Qt::StrongFocus);
        audit.setRoot(root);
        QCOMPARE(audit.root(), root);

        audit.scan();
        for (const QVariant &entry : audit.issues()) {
            const QVariantMap issue = entry.toMap();
            QVERIFY(!issue.contains(QStringLiteral("item")));
            QVERIFY(issue.contains(QStringLiteral("path")));
            QVERIFY(issue.contains(QStringLiteral("className")));
        }

        QSignalSpy rootChanged(&audit, &AccessibilityAudit::rootChanged);
        delete root;

        QCOMPARE(audit.root(), nullptr);
        QCOMPARE(audit.issueCount(), 0);
        QCOMPARE(rootChanged.count(), 1);
    }

    void exportsJsonAtomicallyAndReportsFailures()
    {
        DiagnosticsModel model;
        model.setMetric(QStringLiteral("samples.count"), 42);
        model.recordEvent(QStringLiteral("scanner"),
                          QStringLiteral("accepted"),
                          {{QStringLiteral("length"), 8}},
                          QStringLiteral("info"),
                          QStringLiteral("Synthetic scan accepted"));

        QVERIFY(!model.exportJson(QString()));
        QVERIFY(!model.lastExportError().isEmpty());

        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("diagnostics.json"));
        QVERIFY(model.exportJson(path));
        QVERIFY(model.lastExportError().isEmpty());

        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
        QCOMPARE(parseError.error, QJsonParseError::NoError);
        QVERIFY(document.isObject());
        QCOMPARE(document.object().value(QStringLiteral("schemaVersion")).toInt(), 1);
        QCOMPARE(document.object().value(QStringLiteral("metrics")).toObject()
                     .value(QStringLiteral("samples.count")).toInt(), 42);
        QCOMPARE(document.object().value(QStringLiteral("events")).toArray().size(), 1);
    }

    void resetClearsEventsMetricsAndDroppedState()
    {
        DiagnosticsModel model;
        model.setMaxEvents(1);
        model.recordEvent(QStringLiteral("test"), QStringLiteral("one"));
        model.recordEvent(QStringLiteral("test"), QStringLiteral("two"));
        model.setMetric(QStringLiteral("value"), 5);
        QCOMPARE(model.droppedCount(), 1);

        model.reset();
        QCOMPARE(model.count(), 0);
        QCOMPARE(model.droppedCount(), 0);
        QVERIFY(model.metrics().isEmpty());

        model.recordEvent(QStringLiteral("test"), QStringLiteral("after-reset"));
        QCOMPARE(model.eventAt(0).value(QStringLiteral("sequence")).toLongLong(), 1);
    }
};

QTEST_MAIN(DiagnosticsTests)
#include "tst_diagnostics.moc"
