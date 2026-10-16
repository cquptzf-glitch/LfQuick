#pragma once

#include <LfQuick/Charts/ChartDataModel.h>
#include <LfQuick/Charts/ChartDescriptor.h>
#include <LfQuick/Charts/LfQuickChartsGlobal.h>

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>

#include <memory>
#include <optional>

namespace LfQuick {

class LFQUICKCHARTS_EXPORT SeriesModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        XAxisIdRole,
        YAxisIdRole,
        VisibleRole,
        MetadataRole,
        PointCountRole,
        DataModelRole
    };
    Q_ENUM(Role)

    explicit SeriesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const noexcept { return m_series.size(); }

    Q_INVOKABLE bool appendSeries(const QVariantMap &descriptor);
    Q_INVOKABLE bool removeSeries(const QString &id);
    Q_INVOKABLE void clear();
    Q_INVOKABLE int rowForSeries(const QString &id) const;
    Q_INVOKABLE QVariantMap seriesAt(int row) const;
    Q_INVOKABLE QObject *dataModelForSeries(const QString &id) const;
    Q_INVOKABLE bool setSeriesVisible(const QString &id, bool visible);
    Q_INVOKABLE bool setPoints(const QString &id, const QVariantList &points);
    Q_INVOKABLE bool appendPoint(const QString &id, double x, double y,
                                 const QVariantMap &metadata = {});

signals:
    void countChanged();

private:
    struct Entry {
        SeriesDescriptor descriptor;
        std::unique_ptr<ChartDataModel> data;
    };

    static ChartSeriesType typeFromVariant(const QVariant &value);
    static QString typeName(ChartSeriesType type);
    static std::optional<SeriesDescriptor> descriptorFromMap(const QVariantMap &descriptor);
    QVariantMap entryToMap(const Entry &entry, int row) const;

    QList<std::shared_ptr<Entry>> m_series;
};

} // namespace LfQuick
