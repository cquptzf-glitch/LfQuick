#include <LfQuick/DataView/DataFilterProxyModel.h>

#include <QAbstractItemModel>

namespace LfQuick {

DataFilterProxyModel::DataFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    connect(this, &QAbstractItemModel::rowsInserted,
            this, [this] { emit countChanged(); });
    connect(this, &QAbstractItemModel::rowsRemoved,
            this, [this] { emit countChanged(); });
    connect(this, &QAbstractItemModel::modelReset,
            this, [this] { emit countChanged(); });
    connect(this, &QAbstractItemModel::layoutChanged,
            this, [this] { emit countChanged(); });
}

QString DataFilterProxyModel::filterText() const
{
    return m_filterText;
}

void DataFilterProxyModel::setFilterText(const QString &text)
{
    if (m_filterText == text)
        return;
    m_filterText = text;
    invalidateFilter();
    emit filterTextChanged();
}

QString DataFilterProxyModel::filterKey() const
{
    return m_filterKey;
}

void DataFilterProxyModel::setFilterKey(const QString &key)
{
    if (m_filterKey == key)
        return;
    m_filterKey = key;
    invalidateFilter();
    emit filterKeyChanged();
}

bool DataFilterProxyModel::caseSensitive() const
{
    return m_caseSensitive;
}

void DataFilterProxyModel::setCaseSensitive(bool value)
{
    if (m_caseSensitive == value)
        return;
    m_caseSensitive = value;
    invalidateFilter();
    emit caseSensitiveChanged();
}

bool DataFilterProxyModel::exactMatch() const
{
    return m_exactMatch;
}

void DataFilterProxyModel::setExactMatch(bool value)
{
    if (m_exactMatch == value)
        return;
    m_exactMatch = value;
    invalidateFilter();
    emit exactMatchChanged();
}

int DataFilterProxyModel::mapRowToSource(int proxyRow) const
{
    if (proxyRow < 0 || proxyRow >= rowCount())
        return -1;
    const QModelIndex source = mapToSource(index(proxyRow, 0));
    return source.isValid() ? source.row() : -1;
}

int DataFilterProxyModel::mapRowFromSource(int sourceRow) const
{
    if (!sourceModel() || sourceRow < 0 || sourceRow >= sourceModel()->rowCount())
        return -1;
    const QModelIndex proxy = mapFromSource(sourceModel()->index(sourceRow, 0));
    return proxy.isValid() ? proxy.row() : -1;
}

bool DataFilterProxyModel::matches(const QString &value) const
{
    const auto sensitivity = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    return m_exactMatch
        ? value.compare(m_filterText, sensitivity) == 0
        : value.contains(m_filterText, sensitivity);
}

bool DataFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty())
        return true;

    const auto *model = sourceModel();
    if (!model)
        return false;

    if (!m_filterKey.isEmpty()) {
        const auto roles = model->roleNames();
        for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
            if (QString::fromUtf8(it.value()) != m_filterKey)
                continue;
            const auto value = model->data(model->index(sourceRow, 0, sourceParent), it.key());
            return matches(value.toString());
        }
        return false;
    }

    for (int column = 0; column < model->columnCount(sourceParent); ++column) {
        const auto value = model->data(model->index(sourceRow, column, sourceParent), Qt::DisplayRole);
        if (matches(value.toString()))
            return true;
    }

    return false;
}

} // namespace LfQuick
