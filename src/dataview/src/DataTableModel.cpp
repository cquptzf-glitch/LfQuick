#include <LfQuick/DataView/DataTableModel.h>

#include <QRegularExpression>

#include <algorithm>
#include <utility>

namespace LfQuick {

namespace {
constexpr int FirstCustomRole = Qt::UserRole + 1;

bool isValidRoleName(const QString &key)
{
    static const QRegularExpression expression(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_]*$"));
    return expression.match(key).hasMatch();
}

} // namespace

DataTableModel::DataTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    rebuildRoles();
}

QStringList DataTableModel::columnKeys() const
{
    return m_columnKeys;
}

void DataTableModel::setColumnKeys(const QStringList &keys)
{
    QStringList normalized;
    for (const auto &key : keys) {
        if (key.isEmpty() || !isValidRoleName(key) || normalized.contains(key))
            continue;
        normalized.push_back(key);
    }

    if (m_columnKeys == normalized)
        return;

    beginResetModel();
    m_columnKeys = std::move(normalized);
    rebuildRoles();
    endResetModel();
    emit columnKeysChanged();
}

int DataTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

int DataTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_columnKeys.size();
}

QVariant DataTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()
        || index.column() < 0 || index.column() >= m_columnKeys.size()) {
        return {};
    }

    const auto &row = m_rows.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return row.value(m_columnKeys.at(index.column()));

    const auto key = keyForRole(role);
    return key.isEmpty() ? QVariant{} : row.value(key);
}

bool DataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()
        || index.column() < 0 || index.column() >= m_columnKeys.size()) {
        return false;
    }

    const QString key = (role == Qt::DisplayRole || role == Qt::EditRole)
        ? m_columnKeys.at(index.column())
        : keyForRole(role);

    if (key.isEmpty())
        return false;

    if (m_rows[index.row()].value(key) == value)
        return true;

    m_rows[index.row()][key] = value;
    QList<int> changedRoles{Qt::DisplayRole, Qt::EditRole};
    if (role >= Qt::UserRole)
        changedRoles.push_back(role);
    emit dataChanged(index, index, changedRoles);
    return true;
}

Qt::ItemFlags DataTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index)
        | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal && section >= 0 && section < m_columnKeys.size())
        return m_columnKeys.at(section);

    if (orientation == Qt::Vertical && section >= 0 && section < m_rows.size())
        return section + 1;

    return {};
}

QHash<int, QByteArray> DataTableModel::roleNames() const
{
    return m_roles;
}

void DataTableModel::clear()
{
    if (m_rows.isEmpty())
        return;

    beginResetModel();
    m_rows.clear();
    endResetModel();
    emit countChanged();
}

void DataTableModel::setRows(const QVariantList &rows)
{
    QVector<QVariantMap> next;
    next.reserve(rows.size());
    for (const auto &value : rows) {
        if (value.canConvert<QVariantMap>())
            next.push_back(value.toMap());
    }

    beginResetModel();
    m_rows = std::move(next);
    endResetModel();
    emit countChanged();
}

void DataTableModel::appendRow(const QVariantMap &value)
{
    const int row = m_rows.size();
    beginInsertRows({}, row, row);
    m_rows.push_back(value);
    endInsertRows();
    emit countChanged();
}

void DataTableModel::insertRow(int row, const QVariantMap &value)
{
    row = qBound(0, row, m_rows.size());
    beginInsertRows({}, row, row);
    m_rows.insert(row, value);
    endInsertRows();
    emit countChanged();
}

void DataTableModel::removeRow(int row)
{
    if (row < 0 || row >= m_rows.size())
        return;

    beginRemoveRows({}, row, row);
    m_rows.removeAt(row);
    endRemoveRows();
    emit countChanged();
}

QVariantMap DataTableModel::rowAt(int row) const
{
    if (row < 0 || row >= m_rows.size())
        return {};
    return m_rows.at(row);
}

bool DataTableModel::setCell(int row, const QString &key, const QVariant &value)
{
    const int role = roleForKey(key);
    const int column = m_columnKeys.indexOf(key);
    if (role < 0 || column < 0 || row < 0 || row >= m_rows.size())
        return false;

    return setData(index(row, column), value, role);
}

void DataTableModel::sortByKey(const QString &key, Qt::SortOrder order)
{
    if (!m_columnKeys.contains(key) || m_rows.size() < 2)
        return;

    beginResetModel();

    std::stable_sort(m_rows.begin(), m_rows.end(), [&](const QVariantMap &left, const QVariantMap &right) {
        const QVariant l = left.value(key);
        const QVariant r = right.value(key);
        if (l == r)
            return false;

        bool okLeft = false;
        bool okRight = false;
        const double ld = l.toString().toDouble(&okLeft);
        const double rd = r.toString().toDouble(&okRight);

        if (okLeft && okRight)
            return order == Qt::AscendingOrder ? ld < rd : ld > rd;

        const int cmp = QString::localeAwareCompare(l.toString(), r.toString());
        return order == Qt::AscendingOrder ? cmp < 0 : cmp > 0;
    });

    endResetModel();
}

int DataTableModel::roleForKey(const QString &key) const
{
    return m_keyToRole.value(key, -1);
}

QString DataTableModel::keyForRole(int role) const
{
    for (auto it = m_keyToRole.cbegin(); it != m_keyToRole.cend(); ++it) {
        if (it.value() == role)
            return it.key();
    }
    return {};
}

void DataTableModel::rebuildRoles()
{
    m_roles.clear();
    m_keyToRole.clear();

    m_roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
    m_roles.insert(Qt::EditRole, QByteArrayLiteral("edit"));

    int role = FirstCustomRole;
    for (const auto &key : m_columnKeys) {
        if (key == QStringLiteral("display") || key == QStringLiteral("edit"))
            continue;
        m_keyToRole.insert(key, role);
        m_roles.insert(role, key.toUtf8());
        ++role;
    }
}

} // namespace LfQuick
