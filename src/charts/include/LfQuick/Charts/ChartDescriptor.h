#pragma once

#include <LfQuick/Charts/LfQuickChartsGlobal.h>

#include <QString>
#include <QVariantMap>

namespace LfQuick {

enum class ChartSeriesType {
    Line = 0,
    Scatter,
    Bar,
    Area
};

struct LFQUICKCHARTS_EXPORT AxisDescriptor
{
    QString id;
    QString title;
    double minimum = 0.0;
    double maximum = 1.0;
    bool autoRange = true;
    QString labelFormat;
    QVariantMap metadata;
};

struct LFQUICKCHARTS_EXPORT SeriesDescriptor
{
    QString id;
    QString name;
    ChartSeriesType type = ChartSeriesType::Line;
    QString xAxisId;
    QString yAxisId;
    bool visible = true;
    QVariantMap metadata;
};

} // namespace LfQuick
