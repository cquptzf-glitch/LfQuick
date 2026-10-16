#pragma once

#include <LfQuick/Charts/LfQuickChartsGlobal.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QXYSeries>

namespace LfQuick {

class ChartDataModel;

class LFQUICKCHARTS_EXPORT QtGraphsSeriesAdapter final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ChartDataModel* model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(QXYSeries* series READ series WRITE setSeries NOTIFY seriesChanged FINAL)
    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync NOTIFY autoSyncChanged FINAL)

public:
    explicit QtGraphsSeriesAdapter(QObject *parent = nullptr);

    ChartDataModel *model() const { return m_model; }
    void setModel(ChartDataModel *model);

    QXYSeries *series() const { return m_series; }
    void setSeries(QXYSeries *series);

    bool autoSync() const noexcept { return m_autoSync; }
    void setAutoSync(bool enabled);

    Q_INVOKABLE bool sync();

signals:
    void modelChanged();
    void seriesChanged();
    void autoSyncChanged();
    void synced(int pointCount);

private:
    void reconnectModel();
    void disconnectModel();
    void requestSync();

    QPointer<ChartDataModel> m_model;
    QPointer<QXYSeries> m_series;
    bool m_autoSync = true;
    QList<QMetaObject::Connection> m_modelConnections;
};

} // namespace LfQuick
