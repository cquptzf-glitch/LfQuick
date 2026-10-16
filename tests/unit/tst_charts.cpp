#include <LfQuick/Charts/ChartDataModel.h>
#include <LfQuick/Charts/SeriesModel.h>

#include <QtTest>

using namespace LfQuick;

class ChartsTests : public QObject
{
    Q_OBJECT

private slots:
    void chartDataModelTracksPointsAndBounds()
    {
        ChartDataModel model;
        model.setPoints({
            QVariantMap{{QStringLiteral("x"), -2.0},
                        {QStringLiteral("y"), 5.5},
                        {QStringLiteral("metadata"), QVariantMap{{QStringLiteral("tag"), QStringLiteral("a")}}}},
            QVariantMap{{QStringLiteral("x"), 4.0}, {QStringLiteral("y"), -1.0}},
            QVariantMap{{QStringLiteral("x"), QStringLiteral("invalid")}, {QStringLiteral("y"), 2.0}}
        });

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.pointAt(0).value(QStringLiteral("x")).toDouble(), -2.0);
        QCOMPARE(model.pointAt(0).value(QStringLiteral("metadata")).toMap().value(QStringLiteral("tag")).toString(),
                 QStringLiteral("a"));

        const auto initialBounds = model.bounds();
        QVERIFY(initialBounds.value(QStringLiteral("valid")).toBool());
        QCOMPARE(initialBounds.value(QStringLiteral("minX")).toDouble(), -2.0);
        QCOMPARE(initialBounds.value(QStringLiteral("maxX")).toDouble(), 4.0);
        QCOMPARE(initialBounds.value(QStringLiteral("minY")).toDouble(), -1.0);
        QCOMPARE(initialBounds.value(QStringLiteral("maxY")).toDouble(), 5.5);

        model.appendPoint(10.0, 8.0, {{QStringLiteral("source"), QStringLiteral("live")}});
        QCOMPARE(model.count(), 3);
        QCOMPARE(model.bounds().value(QStringLiteral("maxX")).toDouble(), 10.0);
        QCOMPARE(model.bounds().value(QStringLiteral("maxY")).toDouble(), 8.0);

        model.clear();
        QCOMPARE(model.count(), 0);
        QVERIFY(!model.bounds().value(QStringLiteral("valid")).toBool());
    }

    void seriesModelOwnsIndependentPointModels()
    {
        SeriesModel model;
        QVERIFY(model.appendSeries({
            {QStringLiteral("id"), QStringLiteral("signal")},
            {QStringLiteral("name"), QStringLiteral("Signal")},
            {QStringLiteral("type"), QStringLiteral("line")},
            {QStringLiteral("xAxisId"), QStringLiteral("time")},
            {QStringLiteral("yAxisId"), QStringLiteral("value")}
        }));
        QVERIFY(model.appendSeries({
            {QStringLiteral("id"), QStringLiteral("reference")},
            {QStringLiteral("name"), QStringLiteral("Reference")},
            {QStringLiteral("type"), QStringLiteral("scatter")},
            {QStringLiteral("visible"), false}
        }));
        QVERIFY(!model.appendSeries({{QStringLiteral("id"), QStringLiteral("signal")}}));
        QVERIFY(!model.appendSeries({{QStringLiteral("name"), QStringLiteral("Missing id")}}));

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.rowForSeries(QStringLiteral("reference")), 1);
        QCOMPARE(model.seriesAt(0).value(QStringLiteral("type")).toString(), QStringLiteral("line"));
        QCOMPARE(model.seriesAt(1).value(QStringLiteral("type")).toString(), QStringLiteral("scatter"));
        QVERIFY(!model.seriesAt(1).value(QStringLiteral("visible")).toBool());

        QVERIFY(model.setPoints(QStringLiteral("signal"), {
            QVariantMap{{QStringLiteral("x"), 0.0}, {QStringLiteral("y"), 1.0}},
            QVariantMap{{QStringLiteral("x"), 1.0}, {QStringLiteral("y"), 2.0}}
        }));
        QVERIFY(model.appendPoint(QStringLiteral("reference"), 3.0, 4.0));

        QCOMPARE(model.seriesAt(0).value(QStringLiteral("pointCount")).toInt(), 2);
        QCOMPARE(model.seriesAt(1).value(QStringLiteral("pointCount")).toInt(), 1);

        auto *signalData = qobject_cast<ChartDataModel *>(model.dataModelForSeries(QStringLiteral("signal")));
        auto *referenceData = qobject_cast<ChartDataModel *>(model.dataModelForSeries(QStringLiteral("reference")));
        QVERIFY(signalData);
        QVERIFY(referenceData);
        QVERIFY(signalData != referenceData);
        QCOMPARE(signalData->count(), 2);
        QCOMPARE(referenceData->count(), 1);

        QVERIFY(model.setSeriesVisible(QStringLiteral("reference"), true));
        QVERIFY(model.seriesAt(1).value(QStringLiteral("visible")).toBool());
        QVERIFY(!model.setSeriesVisible(QStringLiteral("missing"), true));

        QVERIFY(model.removeSeries(QStringLiteral("signal")));
        QCOMPARE(model.count(), 1);
        QVERIFY(!model.removeSeries(QStringLiteral("missing")));
    }
};

QTEST_MAIN(ChartsTests)
#include "tst_charts.moc"
