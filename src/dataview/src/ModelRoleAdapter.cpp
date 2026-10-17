#include <LfQuick/DataView/ModelRoleAdapter.h>

#include <QAbstractItemModel>

#include <algorithm>

namespace LfQuick {

ModelRoleAdapter::ModelRoleAdapter(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

void ModelRoleAdapter::setRoleMap(const QVariantMap &roleMap)
{
    if (m_roleMap == roleMap)
        return;

    beginResetModel();
    m_roleMap = roleMap;
    rebuildRoleMappings();
    endResetModel();
    emit roleMapChanged();
}

QHash<int, QByteArray> ModelRoleAdapter::roleNames() const
{
    QHash<int, QByteArray> result = sourceModel() ? sourceModel()->roleNames()
                                                   : QHash<int, QByteArray>{};
    for (auto it = m_aliasRoleNames.cbegin(); it != m_aliasRoleNames.cend(); ++it)
        result.insert(it.key(), it.value());
    return result;
}

QVariant ModelRoleAdapter::data(const QModelIndex &proxyIndex, int role) const
{
    const int sourceRole = m_aliasToSourceRole.value(role, role);
    return QIdentityProxyModel::data(proxyIndex, sourceRole);
}

bool ModelRoleAdapter::setData(const QModelIndex &proxyIndex,
                               const QVariant &value,
                               int role)
{
    const int sourceRole = m_aliasToSourceRole.value(role, role);
    return QIdentityProxyModel::setData(proxyIndex, value, sourceRole);
}

void ModelRoleAdapter::setSourceModel(QAbstractItemModel *sourceModelValue)
{
    if (sourceModel() == sourceModelValue)
        return;

    QIdentityProxyModel::setSourceModel(sourceModelValue);
    rebuildRoleMappings();
}

int ModelRoleAdapter::roleForName(const QString &name) const
{
    return m_nameToRole.value(name.trimmed(), -1);
}

QString ModelRoleAdapter::sourceRoleName(const QString &alias) const
{
    const int aliasRole = m_nameToRole.value(alias.trimmed(), -1);
    if (aliasRole < 0)
        return {};

    const int sourceRole = m_aliasToSourceRole.value(aliasRole, aliasRole);
    if (!sourceModel())
        return {};
    return QString::fromUtf8(sourceModel()->roleNames().value(sourceRole));
}

void ModelRoleAdapter::rebuildRoleMappings()
{
    m_aliasToSourceRole.clear();
    m_aliasRoleNames.clear();
    m_nameToRole.clear();

    QHash<int, QByteArray> sourceNames;
    if (sourceModel())
        sourceNames = sourceModel()->roleNames();

    int maximumRole = Qt::UserRole;
    for (auto it = sourceNames.cbegin(); it != sourceNames.cend(); ++it) {
        maximumRole = std::max(maximumRole, it.key());
        m_nameToRole.insert(QString::fromUtf8(it.value()), it.key());
    }

    int nextAliasRole = std::max(Qt::UserRole + 1024, maximumRole + 1);
    for (auto it = m_roleMap.cbegin(); it != m_roleMap.cend(); ++it) {
        const QString alias = it.key().trimmed();
        if (alias.isEmpty())
            continue;

        const int sourceRole = resolveSourceRole(it.value());
        if (sourceRole < 0)
            continue;

        const int aliasRole = nextAliasRole++;
        m_aliasToSourceRole.insert(aliasRole, sourceRole);
        m_aliasRoleNames.insert(aliasRole, alias.toUtf8());
        m_nameToRole.insert(alias, aliasRole);
    }
}

int ModelRoleAdapter::resolveSourceRole(const QVariant &definition) const
{
    if (!sourceModel())
        return -1;

    bool ok = false;
    const int numericRole = definition.toInt(&ok);
    if (ok)
        return numericRole;

    const QString requestedName = definition.toString().trimmed();
    if (requestedName.isEmpty())
        return -1;

    const auto names = sourceModel()->roleNames();
    for (auto it = names.cbegin(); it != names.cend(); ++it) {
        if (QString::fromUtf8(it.value()) == requestedName)
            return it.key();
    }

    if (requestedName.compare(QStringLiteral("display"), Qt::CaseInsensitive) == 0)
        return Qt::DisplayRole;
    if (requestedName.compare(QStringLiteral("edit"), Qt::CaseInsensitive) == 0)
        return Qt::EditRole;
    if (requestedName.compare(QStringLiteral("checkState"), Qt::CaseInsensitive) == 0)
        return Qt::CheckStateRole;
    if (requestedName.compare(QStringLiteral("decoration"), Qt::CaseInsensitive) == 0)
        return Qt::DecorationRole;
    if (requestedName.compare(QStringLiteral("toolTip"), Qt::CaseInsensitive) == 0)
        return Qt::ToolTipRole;

    return -1;
}

} // namespace LfQuick
