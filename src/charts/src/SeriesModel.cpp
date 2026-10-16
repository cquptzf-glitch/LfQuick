#include <LfQuick/Charts/SeriesModel.h>

#include <QMetaType>

namespace LfQuick {

SeriesModel::SeriesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SeriesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_series.size();
}

QVariant SeriesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_series.size())
        return {};

    const auto &entry = *m_series.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return entry.descriptor.name;
    case IdRole:
        return entry.descriptor.id;
    case TypeRole:
        return typeName(entry.descriptor.type);
    case XAxisIdRole:
        return entry.descriptor.xAxisId;
    case YAxisIdRole:
        return entry.descriptor.yAxisId;
    case VisibleRole:
        return entry.descriptor.visible;
    case MetadataRole:
        return entry.descriptor.metadata;
    case PointCountRole:
        return entry.data ? entry.data->count() : 0;
    case DataModelRole:
        return QVariant::fromValue(static_cast<QObject *>(entry.data.get()));
    default:
        return {};
    }
}

QHash<int, QByteArray> SeriesModel::roleNames() const
{
    return {
        {IdRole, "seriesId"},
        {NameRole, "name"},
        {TypeRole, "seriesType"},
        {XAxisIdRole, "xAxisId"},
        {YAxisIdRole, "yAxisId"},
        {VisibleRole, "seriesVisible"},
        {MetadataRole, "metadata"},
        {PointCountRole, "pointCount"},
        {DataModelRole, "dataModel"}
    };
}

bool SeriesModel::appendSeries(const QVariantMap &descriptor)
{
    auto parsed = descriptorFromMap(descriptor);
    if (!parsed.has_value() || rowForSeries(parsed->id) >= 0)
        return false;

    auto entry = std::make_shared<Entry>();
    entry->descriptor = std::move(*parsed);
    entry->data = std::make_unique<ChartDataModel>();

    const int row = m_series.size();
    beginInsertRows({}, row, row);
    m_series.push_back(std::move(entry));
    endInsertRows();
    emit countChanged();
    return true;
}

bool SeriesModel::removeSeries(const QString &id)
{
    const int row = rowForSeries(id);
    if (row < 0)
        return false;

    beginRemoveRows({}, row, row);
    m_series.removeAt(row);
    endRemoveRows();
    emit countChanged();
    return true;
}

void SeriesModel::clear()
{
    if (m_series.isEmpty())
        return;

    beginResetModel();
    m_series.clear();
    endResetModel();
    emit countChanged();
}

int SeriesModel::rowForSeries(const QString &id) const
{
    if (id.isEmpty())
        return -1;
    for (int row = 0; row < m_series.size(); ++row) {
        if (m_series.at(row)->descriptor.id == id)
            return row;
    }
    return -1;
}

QVariantMap SeriesModel::seriesAt(int row) const
{
    if (row < 0 || row >= m_series.size())
        return {};
    return entryToMap(*m_series.at(row), row);
}

QObject *SeriesModel::dataModelForSeries(const QString &id) const
{
    const int row = rowForSeries(id);
    if (row < 0)
        return nullptr;
    return m_series.at(row)->data.get();
}

bool SeriesModel::setSeriesVisible(const QString &id, bool visible)
{
    const int row = rowForSeries(id);
    if (row < 0)
        return false;

    auto &entry = *m_series[row];
    if (entry.descriptor.visible == visible)
        return true;
    entry.descriptor.visible = visible;
    emit dataChanged(index(row, 0), index(row, 0), {VisibleRole});
    return true;
}

bool SeriesModel::setPoints(const QString &id, const QVariantList &points)
{
    const int row = rowForSeries(id);
    if (row < 0)
        return false;
    m_series.at(row)->data->setPoints(points);
    emit dataChanged(index(row, 0), index(row, 0), {PointCountRole});
    return true;
}

bool SeriesModel::appendPoint(const QString &id, double x, double y,
                              const QVariantMap &metadata)
{
    const int row = rowForSeries(id);
    if (row < 0)
        return false;
    m_series.at(row)->data->appendPoint(x, y, metadata);
    emit dataChanged(index(row, 0), index(row, 0), {PointCountRole});
    return true;
}

ChartSeriesType SeriesModel::typeFromVariant(const QVariant &value)
{
    const QString name = value.toString().trimmed().toLower();
    if (name == QStringLiteral("scatter"))
        return ChartSeriesType::Scatter;
    if (name == QStringLiteral("bar"))
        return ChartSeriesType::Bar;
    if (name == QStringLiteral("area"))
        return ChartSeriesType::Area;
    if (name == QStringLiteral("line"))
        return ChartSeriesType::Line;

    const int typeId = value.metaType().id();
    if (typeId == QMetaType::Int || typeId == QMetaType::UInt
        || typeId == QMetaType::LongLong || typeId == QMetaType::ULongLong) {
        switch (value.toInt()) {
        case static_cast<int>(ChartSeriesType::Scatter):
            return ChartSeriesType::Scatter;
        case static_cast<int>(ChartSeriesType::Bar):
            return ChartSeriesType::Bar;
        case static_cast<int>(ChartSeriesType::Area):
            return ChartSeriesType::Area;
        default:
            break;
        }
    }
    return ChartSeriesType::Line;
}

QString SeriesModel::typeName(ChartSeriesType type)
{
    switch (type) {
    case ChartSeriesType::Scatter:
        return QStringLiteral("scatter");
    case ChartSeriesType::Bar:
        return QStringLiteral("bar");
    case ChartSeriesType::Area:
        return QStringLiteral("area");
    case ChartSeriesType::Line:
    default:
        return QStringLiteral("line");
    }
}

std::optional<SeriesDescriptor> SeriesModel::descriptorFromMap(const QVariantMap &descriptor)
{
    SeriesDescriptor parsed;
    parsed.id = descriptor.value(QStringLiteral("id")).toString().trimmed();
    if (parsed.id.isEmpty())
        return std::nullopt;

    parsed.name = descriptor.value(QStringLiteral("name"), parsed.id).toString();
    parsed.type = typeFromVariant(descriptor.value(QStringLiteral("type")));
    parsed.xAxisId = descriptor.value(QStringLiteral("xAxisId")).toString();
    parsed.yAxisId = descriptor.value(QStringLiteral("yAxisId")).toString();
    parsed.visible = descriptor.value(QStringLiteral("visible"), true).toBool();
    parsed.metadata = descriptor.value(QStringLiteral("metadata")).toMap();
    return parsed;
}

QVariantMap SeriesModel::entryToMap(const Entry &entry, int row) const
{
    return {
        {QStringLiteral("index"), row},
        {QStringLiteral("id"), entry.descriptor.id},
        {QStringLiteral("name"), entry.descriptor.name},
        {QStringLiteral("type"), typeName(entry.descriptor.type)},
        {QStringLiteral("xAxisId"), entry.descriptor.xAxisId},
        {QStringLiteral("yAxisId"), entry.descriptor.yAxisId},
        {QStringLiteral("visible"), entry.descriptor.visible},
        {QStringLiteral("metadata"), entry.descriptor.metadata},
        {QStringLiteral("pointCount"), entry.data ? entry.data->count() : 0},
        {QStringLiteral("dataModel"), QVariant::fromValue(static_cast<QObject *>(entry.data.get()))}
    };
}

} // namespace LfQuick
