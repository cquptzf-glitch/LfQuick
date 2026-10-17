#pragma once

#include <LfQuick/Charts/ChartBackendRegistry.h>
#include <LfQuick/Charts/ChartDescriptor.h>
#include <LfQuick/QtAdapters/LfQuickLegacyQCustomPlotGlobal.h>

#include <QAbstractItemModel>
#include <QObject>
#include <QPointer>
#include <QWidget>

#include <functional>

namespace LfQuick {

class LFQUICKLEGACYQCP_EXPORT LegacyQCustomPlotBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availabilityChanged FINAL)

public:
    using WidgetFactory = std::function<QWidget *(QWidget *parent)>;
    using ModelBinder = std::function<bool(QWidget *widget,
                                           QAbstractItemModel *model,
                                           const ChartDescriptor &descriptor)>;

    explicit LegacyQCustomPlotBackend(QObject *parent = nullptr);

    bool available() const noexcept { return static_cast<bool>(m_factory) && static_cast<bool>(m_binder); }

    void setFactory(WidgetFactory factory);
    void setBinder(ModelBinder binder);

    ChartBackendDescriptor backendDescriptor() const;

    QWidget *create(QWidget *parent,
                    QAbstractItemModel *model,
                    const ChartDescriptor &descriptor) const;
    bool update(QWidget *widget,
                QAbstractItemModel *model,
                const ChartDescriptor &descriptor) const;

signals:
    void availabilityChanged();

private:
    WidgetFactory m_factory;
    ModelBinder m_binder;
};

} // namespace LfQuick
