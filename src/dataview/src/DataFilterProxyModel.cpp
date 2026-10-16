#include <LfQuick/DataView/DataFilterProxyModel.h>

#include <QAbstractItemModel>

namespace LfQuick {

DataFilterProxyModel::DataFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
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

bool DataFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty())
        return true;

    const auto *model = sourceModel();
    if (!model)
        return false;

    const auto flags = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (!m_filterKey.isEmpty()) {
        const auto roles = model->roleNames();
        for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
            if (QString::fromUtf8(it.value()) != m_filterKey)
                continue;
            const auto value = model->data(model->index(sourceRow, 0, sourceParent), it.key());
            return value.toString().contains(m_filterText, flags);
        }
        return false;
    }

    for (int column = 0; column < model->columnCount(sourceParent); ++column) {
        const auto value = model->data(model->index(sourceRow, column, sourceParent), Qt::DisplayRole);
        if (value.toString().contains(m_filterText, flags))
            return true;
    }

    return false;
}

} // namespace LfQuick
