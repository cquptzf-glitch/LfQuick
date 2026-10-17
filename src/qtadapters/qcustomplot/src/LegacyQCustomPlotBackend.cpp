#include <LfQuick/QtAdapters/LegacyQCustomPlotBackend.h>

#include <utility>

namespace LfQuick {

LegacyQCustomPlotBackend::LegacyQCustomPlotBackend(QObject *parent)
    : QObject(parent)
{
}

void LegacyQCustomPlotBackend::setFactory(WidgetFactory factory)
{
    const bool wasAvailable = available();
    m_factory = std::move(factory);
    if (wasAvailable != available())
        emit availabilityChanged();
}

void LegacyQCustomPlotBackend::setBinder(ModelBinder binder)
{
    const bool wasAvailable = available();
    m_binder = std::move(binder);
    if (wasAvailable != available())
        emit availabilityChanged();
}

ChartBackendDescriptor LegacyQCustomPlotBackend::backendDescriptor() const
{
    ChartBackendDescriptor descriptor;
    descriptor.id = QStringLiteral("legacy-qcustomplot");
    descriptor.name = QStringLiteral("Legacy QCustomPlot");
    descriptor.technology = QStringLiteral("qwidget");
    descriptor.priority = 40;
    descriptor.available = available();
    descriptor.seriesTypes = {
        ChartSeriesType::Line,
        ChartSeriesType::Scatter,
        ChartSeriesType::Bar,
        ChartSeriesType::Area
    };
    descriptor.metadata.insert(QStringLiteral("factoryBased"), true);
    descriptor.metadata.insert(QStringLiteral("externalDependency"), QStringLiteral("QCustomPlot"));
    return descriptor;
}

QWidget *LegacyQCustomPlotBackend::create(QWidget *parent,
                                          QAbstractItemModel *model,
                                          const ChartDescriptor &descriptor) const
{
    if (!available())
        return nullptr;

    QWidget *widget = m_factory(parent);
    if (!widget)
        return nullptr;

    if (!m_binder(widget, model, descriptor)) {
        delete widget;
        return nullptr;
    }
    return widget;
}

bool LegacyQCustomPlotBackend::update(QWidget *widget,
                                      QAbstractItemModel *model,
                                      const ChartDescriptor &descriptor) const
{
    return widget && m_binder && m_binder(widget, model, descriptor);
}

} // namespace LfQuick
