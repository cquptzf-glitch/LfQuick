#include <LfQuick/Charts/QtGraphsSeriesAdapter.h>

#include <LfQuick/Charts/ChartDataModel.h>

#include <QAbstractItemModel>

#include <utility>

namespace LfQuick {

QtGraphsSeriesAdapter::QtGraphsSeriesAdapter(QObject *parent)
    : QObject(parent)
{
}

void QtGraphsSeriesAdapter::setModel(ChartDataModel *model)
{
    if (m_model == model)
        return;

    disconnectModel();
    m_model = model;
    reconnectModel();
    emit modelChanged();
    requestSync();
}

void QtGraphsSeriesAdapter::setSeries(QXYSeries *series)
{
    if (m_series == series)
        return;

    if (m_series)
        disconnect(m_series, nullptr, this, nullptr);

    m_series = series;
    if (m_series) {
        connect(m_series, &QObject::destroyed, this, [this] {
            m_series = nullptr;
            emit seriesChanged();
        });
    }

    emit seriesChanged();
    requestSync();
}

void QtGraphsSeriesAdapter::setAutoSync(bool enabled)
{
    if (m_autoSync == enabled)
        return;
    m_autoSync = enabled;
    emit autoSyncChanged();
    if (m_autoSync)
        sync();
}

bool QtGraphsSeriesAdapter::sync()
{
    if (!m_model || !m_series)
        return false;

    const auto points = m_model->points();
    m_series->replace(points);
    emit synced(points.size());
    return true;
}

void QtGraphsSeriesAdapter::reconnectModel()
{
    if (!m_model)
        return;

    m_modelConnections.append(connect(m_model, &QAbstractItemModel::modelReset,
                                      this, &QtGraphsSeriesAdapter::requestSync));
    m_modelConnections.append(connect(m_model, &QAbstractItemModel::rowsInserted,
                                      this, [this] { requestSync(); }));
    m_modelConnections.append(connect(m_model, &QAbstractItemModel::rowsRemoved,
                                      this, [this] { requestSync(); }));
    m_modelConnections.append(connect(m_model, &QAbstractItemModel::dataChanged,
                                      this, [this] { requestSync(); }));
    m_modelConnections.append(connect(m_model, &QObject::destroyed, this, [this] {
        m_model = nullptr;
        m_modelConnections.clear();
        emit modelChanged();
    }));
}

void QtGraphsSeriesAdapter::disconnectModel()
{
    for (const auto &connection : std::as_const(m_modelConnections))
        disconnect(connection);
    m_modelConnections.clear();
}

void QtGraphsSeriesAdapter::requestSync()
{
    if (m_autoSync)
        sync();
}

} // namespace LfQuick
