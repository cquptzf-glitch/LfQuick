#include <LfQuick/DataView/ColumnStateModel.h>

#include <QSet>

#include <algorithm>

namespace LfQuick {

ColumnStateModel::ColumnStateModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ColumnStateModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_columns.size();
}

QVariant ColumnStateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_columns.size())
        return {};

    const auto &column = m_columns.at(index.row());
    switch (role) {
    case IdRole: return column.id;
    case ModelColumnRole: return column.modelColumn;
    case TitleRole: return column.title;
    case WidthRole: return column.width;
    case VisibleRole: return column.visible;
    case OrderRole: return index.row();
    default: return {};
    }
}

QHash<int, QByteArray> ColumnStateModel::roleNames() const
{
    return {
        {IdRole, QByteArrayLiteral("columnId")},
        {ModelColumnRole, QByteArrayLiteral("modelColumn")},
        {TitleRole, QByteArrayLiteral("title")},
        {WidthRole, QByteArrayLiteral("width")},
        {VisibleRole, QByteArrayLiteral("visible")},
        {OrderRole, QByteArrayLiteral("order")}
    };
}

void ColumnStateModel::clear()
{
    if (m_columns.isEmpty())
        return;
    beginResetModel();
    m_columns.clear();
    endResetModel();
    emit countChanged();
    emit stateChanged();
}

bool ColumnStateModel::parseColumn(const QVariantMap &map, ColumnState &state)
{
    state.id = map.value(QStringLiteral("id")).toString().trimmed();
    state.modelColumn = map.value(QStringLiteral("modelColumn"), -1).toInt();
    state.title = map.value(QStringLiteral("title"), state.id).toString();
    state.width = std::max<qreal>(24.0, map.value(QStringLiteral("width"), 140.0).toReal());
    state.visible = map.value(QStringLiteral("visible"), true).toBool();
    return !state.id.isEmpty() && state.modelColumn >= 0;
}

bool ColumnStateModel::setColumns(const QVariantList &columns)
{
    QList<ColumnState> next;
    QSet<QString> ids;
    QSet<int> modelColumns;
    next.reserve(columns.size());

    for (const QVariant &value : columns) {
        ColumnState state;
        if (!parseColumn(value.toMap(), state) || ids.contains(state.id)
            || modelColumns.contains(state.modelColumn)) {
            return false;
        }
        ids.insert(state.id);
        modelColumns.insert(state.modelColumn);
        next.push_back(std::move(state));
    }

    beginResetModel();
    const bool countChangedValue = m_columns.size() != next.size();
    m_columns = std::move(next);
    endResetModel();
    if (countChangedValue)
        emit countChanged();
    emit stateChanged();
    return true;
}

QVariantList ColumnStateModel::snapshot() const
{
    QVariantList result;
    result.reserve(m_columns.size());
    for (int i = 0; i < m_columns.size(); ++i) {
        const auto &column = m_columns.at(i);
        result.push_back(QVariantMap{
            {QStringLiteral("id"), column.id},
            {QStringLiteral("modelColumn"), column.modelColumn},
            {QStringLiteral("title"), column.title},
            {QStringLiteral("width"), column.width},
            {QStringLiteral("visible"), column.visible},
            {QStringLiteral("order"), i}
        });
    }
    return result;
}

bool ColumnStateModel::restoreState(const QVariantList &columns)
{
    if (columns.isEmpty() || m_columns.isEmpty())
        return false;

    struct SavedState {
        qreal width = 140.0;
        bool visible = true;
        int order = 0;
    };
    QHash<QString, SavedState> saved;
    int fallbackOrder = 0;
    for (const QVariant &value : columns) {
        const QVariantMap map = value.toMap();
        const QString id = map.value(QStringLiteral("id")).toString();
        if (id.isEmpty() || saved.contains(id))
            continue;
        SavedState state;
        state.width = std::max<qreal>(24.0, map.value(QStringLiteral("width"), 140.0).toReal());
        state.visible = map.value(QStringLiteral("visible"), true).toBool();
        state.order = map.value(QStringLiteral("order"), fallbackOrder).toInt();
        saved.insert(id, state);
        ++fallbackOrder;
    }

    QList<ColumnState> restored = m_columns;
    for (auto &column : restored) {
        const auto it = saved.constFind(column.id);
        if (it == saved.cend())
            continue;
        column.width = it->width;
        column.visible = it->visible;
    }

    std::stable_sort(restored.begin(), restored.end(), [&](const ColumnState &left, const ColumnState &right) {
        const bool leftSaved = saved.contains(left.id);
        const bool rightSaved = saved.contains(right.id);
        if (leftSaved != rightSaved)
            return leftSaved;
        if (!leftSaved)
            return false;
        return saved.value(left.id).order < saved.value(right.id).order;
    });

    beginResetModel();
    m_columns = std::move(restored);
    endResetModel();
    emit stateChanged();
    return true;
}

int ColumnStateModel::indexOfId(const QString &id) const
{
    for (int i = 0; i < m_columns.size(); ++i) {
        if (m_columns.at(i).id == id)
            return i;
    }
    return -1;
}

int ColumnStateModel::indexOfModelColumn(int modelColumn) const
{
    for (int i = 0; i < m_columns.size(); ++i) {
        if (m_columns.at(i).modelColumn == modelColumn)
            return i;
    }
    return -1;
}

int ColumnStateModel::modelColumnAt(int visualIndex) const
{
    if (visualIndex < 0 || visualIndex >= m_columns.size())
        return -1;
    return m_columns.at(visualIndex).modelColumn;
}

QString ColumnStateModel::idForModelColumn(int modelColumn) const
{
    const int index = indexOfModelColumn(modelColumn);
    return index < 0 ? QString{} : m_columns.at(index).id;
}

QString ColumnStateModel::titleForModelColumn(int modelColumn) const
{
    const int index = indexOfModelColumn(modelColumn);
    return index < 0 ? QString{} : m_columns.at(index).title;
}

qreal ColumnStateModel::widthForModelColumn(int modelColumn, qreal fallback) const
{
    const int index = indexOfModelColumn(modelColumn);
    return index < 0 ? fallback : m_columns.at(index).width;
}

bool ColumnStateModel::isModelColumnVisible(int modelColumn) const
{
    const int index = indexOfModelColumn(modelColumn);
    return index < 0 ? true : m_columns.at(index).visible;
}

bool ColumnStateModel::setWidth(const QString &id, qreal width)
{
    const int index = indexOfId(id);
    if (index < 0)
        return false;
    width = std::max<qreal>(24.0, width);
    if (qFuzzyCompare(m_columns[index].width, width))
        return true;
    m_columns[index].width = width;
    emit dataChanged(this->index(index), this->index(index), {WidthRole});
    emit stateChanged();
    return true;
}

bool ColumnStateModel::setWidthByModelColumn(int modelColumn, qreal width)
{
    const int index = indexOfModelColumn(modelColumn);
    return index >= 0 && setWidth(m_columns.at(index).id, width);
}

bool ColumnStateModel::setVisible(const QString &id, bool visible)
{
    const int index = indexOfId(id);
    if (index < 0)
        return false;
    if (m_columns[index].visible == visible)
        return true;
    m_columns[index].visible = visible;
    emit dataChanged(this->index(index), this->index(index), {VisibleRole});
    emit stateChanged();
    return true;
}

bool ColumnStateModel::moveModelColumn(int modelColumn, int visualIndex)
{
    const int from = indexOfModelColumn(modelColumn);
    if (from < 0 || m_columns.isEmpty())
        return false;
    visualIndex = qBound(0, visualIndex, m_columns.size() - 1);
    if (from == visualIndex)
        return true;

    const int destination = from < visualIndex ? visualIndex + 1 : visualIndex;
    beginMoveRows({}, from, from, {}, destination);
    m_columns.move(from, visualIndex);
    endMoveRows();
    emit dataChanged(index(0), index(m_columns.size() - 1), {OrderRole});
    emit stateChanged();
    return true;
}

} // namespace LfQuick
