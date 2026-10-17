#include <LfQuick/Diagnostics/DiagnosticsModel.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

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
