#include <LfQuick/Charts/ChartDataModel.h>
#include <LfQuick/Charts/SeriesModel.h>

#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
#include <LfQuick/Charts/QtGraphsAxisAdapter.h>
#include <LfQuick/Charts/QtGraphsSeriesAdapter.h>
#include <QLineSeries>
#include <QValueAxis>
#endif

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
        QCOMPARE(model.columnCount(), 2);
        QCOMPARE(model.data(model.index(0, ChartDataModel::XColumn), Qt::DisplayRole).toDouble(), -2.0);
        QCOMPARE(model.data(model.index(0, ChartDataModel::YColumn), Qt::DisplayRole).toDouble(), 5.5);
        QCOMPARE(model.pointAt(0).value(QStringLiteral("metadata")).toMap().value(QStringLiteral("tag")).toString(),
                 QStringLiteral("a"));

        const auto initialBounds = model.bounds();
        QVERIFY(initialBounds.value(QStringLiteral("valid")).toBool());
        QCOMPARE(initialBounds.value(QStringLiteral("minX")).toDouble(), -2.0);
        QCOMPARE(initialBounds.value(QStringLiteral("maxX")).toDouble(), 4.0);
        QCOMPARE(initialBounds.value(QStringLiteral("minY")).toDouble(), -1.0);
        QCOMPARE(initialBounds.value(QStringLiteral("maxY")).toDouble(), 5.5);

        QVERIFY(model.setData(model.index(0, ChartDataModel::YColumn), 7.25));
        QCOMPARE(model.pointAt(0).value(QStringLiteral("y")).toDouble(), 7.25);
        QVERIFY(!model.setData(model.index(0, ChartDataModel::XColumn), QStringLiteral("not-a-number")));

        model.appendPoint(10.0, 8.0, {{QStringLiteral("source"), QStringLiteral("live")}});
        QCOMPARE(model.count(), 3);
        QCOMPARE(model.bounds().value(QStringLiteral("maxX")).toDouble(), 10.0);
        QCOMPARE(model.bounds().value(QStringLiteral("maxY")).toDouble(), 8.0);
        QCOMPARE(model.points().size(), 3);

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

#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
    void qtGraphsSeriesAdapterSynchronizesIncrementalChanges()
    {
        ChartDataModel model;
        model.setPoints({
            QVariantMap{{QStringLiteral("x"), 0.0}, {QStringLiteral("y"), 1.0}},
            QVariantMap{{QStringLiteral("x"), 1.0}, {QStringLiteral("y"), 2.0}}
        });

        QLineSeries series;
        QtGraphsSeriesAdapter adapter;
        QSignalSpy synced(&adapter, &QtGraphsSeriesAdapter::synced);
        adapter.setModel(&model);
        adapter.setSeries(&series);

        QCOMPARE(series.count(), 2);
        QCOMPARE(series.at(0), QPointF(0.0, 1.0));
        QCOMPARE(series.at(1), QPointF(1.0, 2.0));
        QVERIFY(synced.count() >= 1);

        model.appendPoint(2.0, 3.0);
        QCOMPARE(series.count(), 3);
        QCOMPARE(series.at(2), QPointF(2.0, 3.0));

        QVERIFY(model.setData(model.index(0, ChartDataModel::YColumn), 9.0));
        QCOMPARE(series.at(0), QPointF(0.0, 9.0));

        adapter.setAutoSync(false);
        model.appendPoint(3.0, 4.0);
        QCOMPARE(series.count(), 3);
        QVERIFY(adapter.sync());
        QCOMPARE(series.count(), 4);
    }

    void qtGraphsAxisAdapterAppliesDescriptor()
    {
        AxisDescriptor descriptor;
        descriptor.id = QStringLiteral("signal");
        descriptor.title = QStringLiteral("Signal");
        descriptor.minimum = -1.0;
        descriptor.maximum = 5.0;
        descriptor.autoRange = false;
        descriptor.labelFormat = QStringLiteral("%.1f");

        QValueAxis axis;
        QVERIFY(QtGraphsAxisAdapter::apply(descriptor, &axis));
        QCOMPARE(axis.titleText(), QStringLiteral("Signal"));
        QCOMPARE(axis.min(), -1.0);
        QCOMPARE(axis.max(), 5.0);
        QCOMPARE(axis.labelFormat(), QStringLiteral("%.1f"));

        descriptor.maximum = descriptor.minimum;
        QVERIFY(!QtGraphsAxisAdapter::apply(descriptor, &axis));
        QVERIFY(!QtGraphsAxisAdapter::apply(descriptor, nullptr));
    }
#endif
};

QTEST_MAIN(ChartsTests)
#include "tst_charts.moc"
