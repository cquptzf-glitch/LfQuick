#include <LfQuick/Charts/QtGraphsAxisAdapter.h>

#include <QValueAxis>

namespace LfQuick {

bool QtGraphsAxisAdapter::apply(const AxisDescriptor &descriptor, QValueAxis *axis)
{
    if (!axis)
        return false;

    axis->setTitleText(descriptor.title);
    axis->setTitleVisible(!descriptor.title.isEmpty());

    if (!descriptor.labelFormat.isEmpty())
        axis->setLabelFormat(descriptor.labelFormat);

    if (!descriptor.autoRange) {
        if (descriptor.maximum <= descriptor.minimum)
            return false;
        axis->setRange(descriptor.minimum, descriptor.maximum);
    }

    return true;
}

} // namespace LfQuick
