#pragma once

#include <LfQuick/Charts/LfQuickChartsGlobal.h>

#include <QAbstractTableModel>
#include <QPointF>
#include <QVariantList>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKCHARTS_EXPORT ChartDataModel final : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)

public:
    enum Column {
        XColumn = 0,
        YColumn = 1,
        ColumnCount = 2
    };
    Q_ENUM(Column)

    enum Role {
        XRole = Qt::UserRole + 1,
        YRole,
        MetadataRole
    };
    Q_ENUM(Role)

    explicit ChartDataModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const noexcept { return m_points.size(); }

    QList<QPointF> points() const;

    Q_INVOKABLE void setPoints(const QVariantList &points);
    Q_INVOKABLE void appendPoint(double x, double y, const QVariantMap &metadata = {});
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap pointAt(int row) const;
    Q_INVOKABLE QVariantMap bounds() const;

signals:
    void countChanged();
    void boundsChanged();

private:
    struct Point {
        double x = 0.0;
        double y = 0.0;
        QVariantMap metadata;
    };

    QList<Point> m_points;
};

} // namespace LfQuick
