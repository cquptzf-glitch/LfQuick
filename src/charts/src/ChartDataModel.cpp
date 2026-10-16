#include <LfQuick/Charts/ChartDataModel.h>

#include <algorithm>
#include <limits>

namespace LfQuick {

ChartDataModel::ChartDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ChartDataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_points.size();
}

int ChartDataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(ColumnCount);
}

QVariant ChartDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_points.size())
        return {};

    const auto &point = m_points.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == XColumn)
            return point.x;
        if (index.column() == YColumn)
            return point.y;
        return {};
    case XRole:
        return point.x;
    case YRole:
        return point.y;
    case MetadataRole:
        return point.metadata;
    default:
        return {};
    }
}

bool ChartDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_points.size())
        return false;

    bool ok = false;
    const double numericValue = value.toDouble(&ok);
    if (!ok)
        return false;

    auto &point = m_points[index.row()];
    int changedRole = role;
    if (role == Qt::EditRole) {
        if (index.column() == XColumn) {
            point.x = numericValue;
            changedRole = XRole;
        } else if (index.column() == YColumn) {
            point.y = numericValue;
            changedRole = YRole;
        } else {
            return false;
        }
    } else if (role == XRole) {
        point.x = numericValue;
    } else if (role == YRole) {
        point.y = numericValue;
    } else {
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, changedRole});
    emit boundsChanged();
    return true;
}

Qt::ItemFlags ChartDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant ChartDataModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};
    if (section == XColumn)
        return QStringLiteral("X");
    if (section == YColumn)
        return QStringLiteral("Y");
    return {};
}

QHash<int, QByteArray> ChartDataModel::roleNames() const
{
    return {
        {XRole, "x"},
        {YRole, "y"},
        {MetadataRole, "metadata"}
    };
}

QList<QPointF> ChartDataModel::points() const
{
    QList<QPointF> result;
    result.reserve(m_points.size());
    for (const auto &point : m_points)
        result.append(QPointF(point.x, point.y));
    return result;
}

void ChartDataModel::setPoints(const QVariantList &points)
{
    QList<Point> parsed;
    parsed.reserve(points.size());

    for (const auto &value : points) {
        const QVariantMap map = value.toMap();
        bool xOk = false;
        bool yOk = false;
        const double x = map.value(QStringLiteral("x")).toDouble(&xOk);
        const double y = map.value(QStringLiteral("y")).toDouble(&yOk);
        if (!xOk || !yOk)
            continue;
        parsed.push_back({x, y, map.value(QStringLiteral("metadata")).toMap()});
    }

    beginResetModel();
    m_points = std::move(parsed);
    endResetModel();
    emit countChanged();
    emit boundsChanged();
}

void ChartDataModel::appendPoint(double x, double y, const QVariantMap &metadata)
{
    const int row = m_points.size();
    beginInsertRows({}, row, row);
    m_points.push_back({x, y, metadata});
    endInsertRows();
    emit countChanged();
    emit boundsChanged();
}

void ChartDataModel::clear()
{
    if (m_points.isEmpty())
        return;

    beginResetModel();
    m_points.clear();
    endResetModel();
    emit countChanged();
    emit boundsChanged();
}

QVariantMap ChartDataModel::pointAt(int row) const
{
    if (row < 0 || row >= m_points.size())
        return {};
    const auto &point = m_points.at(row);
    return {
        {QStringLiteral("x"), point.x},
        {QStringLiteral("y"), point.y},
        {QStringLiteral("metadata"), point.metadata}
    };
}

QVariantMap ChartDataModel::bounds() const
{
    if (m_points.isEmpty())
        return {{QStringLiteral("valid"), false}};

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto &point : m_points) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
    }

    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("minX"), minX},
        {QStringLiteral("maxX"), maxX},
        {QStringLiteral("minY"), minY},
        {QStringLiteral("maxY"), maxY}
    };
}

} // namespace LfQuick
