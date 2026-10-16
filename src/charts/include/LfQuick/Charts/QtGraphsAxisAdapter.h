#pragma once

#include <LfQuick/Charts/ChartDescriptor.h>
#include <LfQuick/Charts/LfQuickChartsGlobal.h>

class QValueAxis;

namespace LfQuick {

class LFQUICKCHARTS_EXPORT QtGraphsAxisAdapter
{
public:
    static bool apply(const AxisDescriptor &descriptor, QValueAxis *axis);
};

} // namespace LfQuick
