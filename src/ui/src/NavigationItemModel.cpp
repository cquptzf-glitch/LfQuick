#include <LfQuick/Ui/NavigationItemModel.h>

#include <LfQuick/Ui/PermissionEvaluator.h>

#include <QMetaType>

namespace LfQuick {

NavigationItemModel::NavigationItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NavigationItemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant NavigationItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const auto &item = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case LabelRole:
        return item.label;
    case RouteRole:
        return item.route;
    case IconRole:
        return item.icon;
    case SectionRole:
        return sectionName(item.section);
    case GroupRole:
        return item.group;
    case BadgeRole:
        return item.badge;
    case VisibleRole:
        return itemVisible(item);
    case EnabledRole:
        return itemEnabled(item);
    case SelectedRole:
        return item.route == m_selectedRoute;
    case PermissionIdRole:
        return item.permissionId;
    case PermissionContextRole:
        return item.permissionContext;
    case ParametersRole:
        return item.parameters;
    default:
        return {};
    }
}

QHash<int, QByteArray> NavigationItemModel::roleNames() const
{
    return {
        {RouteRole, "route"},
        {LabelRole, "label"},
        {IconRole, "icon"},
        {SectionRole, "section"},
        {GroupRole, "group"},
        {BadgeRole, "badge"},
        {VisibleRole, "visible"},
        {EnabledRole, "enabled"},
        {SelectedRole, "selected"},
        {PermissionIdRole, "permissionId"},
        {PermissionContextRole, "permissionContext"},
        {ParametersRole, "parameters"}
    };
}

void NavigationItemModel::setSelectedRoute(const QString &route)
{
    if (m_selectedRoute == route)
        return;

    const int oldRow = rowForRoute(m_selectedRoute);
    m_selectedRoute = route;
    const int newRow = rowForRoute(m_selectedRoute);

    if (oldRow >= 0)
        emit dataChanged(index(oldRow, 0), index(oldRow, 0), {SelectedRole});
    if (newRow >= 0 && newRow != oldRow)
        emit dataChanged(index(newRow, 0), index(newRow, 0), {SelectedRole});
    emit selectedRouteChanged();
}

void NavigationItemModel::setPermissionEvaluator(PermissionEvaluator *evaluator)
{
    if (m_permissionEvaluator == evaluator)
        return;

    if (m_permissionEvaluator)
        disconnect(m_permissionEvaluator, nullptr, this, nullptr);

    m_permissionEvaluator = evaluator;
    if (m_permissionEvaluator) {
        connect(m_permissionEvaluator, &PermissionEvaluator::providerChanged,
                this, &NavigationItemModel::refreshPermissionRoles);
        connect(m_permissionEvaluator, &QObject::destroyed, this, [this] {
            m_permissionEvaluator = nullptr;
            emit permissionEvaluatorChanged();
            refreshPermissionRoles();
        });
    }

    emit permissionEvaluatorChanged();
    refreshPermissionRoles();
}

bool NavigationItemModel::appendItem(const QVariantMap &item)
{
    auto parsed = itemFromMap(item);
    if (!parsed.has_value() || rowForRoute(parsed->route) >= 0)
        return false;

    const int row = m_items.size();
    beginInsertRows({}, row, row);
    m_items.push_back(std::move(*parsed));
    endInsertRows();
    emit countChanged();
    return true;
}

void NavigationItemModel::setItems(const QVariantList &items)
{
    QList<Item> parsedItems;
    QSet<QString> routes;
    parsedItems.reserve(items.size());

    for (const auto &value : items) {
        const auto parsed = itemFromMap(value.toMap());
        if (!parsed.has_value() || routes.contains(parsed->route))
            continue;
        routes.insert(parsed->route);
        parsedItems.push_back(*parsed);
    }

    beginResetModel();
    m_items = std::move(parsedItems);
    endResetModel();
    emit countChanged();

    if (!m_selectedRoute.isEmpty() && rowForRoute(m_selectedRoute) < 0) {
        m_selectedRoute.clear();
        emit selectedRouteChanged();
    }
}

void NavigationItemModel::clear()
{
    if (m_items.isEmpty())
        return;

    beginResetModel();
    m_items.clear();
    endResetModel();
    emit countChanged();

    if (!m_selectedRoute.isEmpty()) {
        m_selectedRoute.clear();
        emit selectedRouteChanged();
    }
}

QVariantMap NavigationItemModel::get(int row) const
{
    if (row < 0 || row >= m_items.size())
        return {};
    return itemToMap(m_items.at(row), row);
}

int NavigationItemModel::rowForRoute(const QString &route) const
{
    if (route.isEmpty())
        return -1;
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items.at(row).route == route)
            return row;
    }
    return -1;
}

NavigationSection NavigationItemModel::sectionFromVariant(const QVariant &value)
{
    if (value.metaType().id() == QMetaType::Int || value.canConvert<int>()) {
        switch (value.toInt()) {
        case static_cast<int>(NavigationSection::Secondary):
            return NavigationSection::Secondary;
        case static_cast<int>(NavigationSection::Footer):
            return NavigationSection::Footer;
        default:
            break;
        }
    }

    const QString name = value.toString().trimmed().toLower();
    if (name == QStringLiteral("secondary"))
        return NavigationSection::Secondary;
    if (name == QStringLiteral("footer"))
        return NavigationSection::Footer;
    return NavigationSection::Primary;
}

QString NavigationItemModel::sectionName(NavigationSection section)
{
    switch (section) {
    case NavigationSection::Secondary:
        return QStringLiteral("secondary");
    case NavigationSection::Footer:
        return QStringLiteral("footer");
    case NavigationSection::Primary:
    default:
        return QStringLiteral("primary");
    }
}

std::optional<NavigationItemModel::Item> NavigationItemModel::itemFromMap(const QVariantMap &item)
{
    Item parsed;
    parsed.route = item.value(QStringLiteral("route")).toString().trimmed();
    if (parsed.route.isEmpty())
        return std::nullopt;

    parsed.label = item.value(QStringLiteral("label"), parsed.route).toString();
    parsed.icon = item.value(QStringLiteral("icon")).toString();
    parsed.section = sectionFromVariant(item.value(QStringLiteral("section")));
    parsed.group = item.value(QStringLiteral("group")).toString();
    parsed.badge = item.value(QStringLiteral("badge"));
    parsed.visible = item.value(QStringLiteral("visible"), true).toBool();
    parsed.enabled = item.value(QStringLiteral("enabled"), true).toBool();
    parsed.permissionId = item.value(QStringLiteral("permissionId")).toString().trimmed();
    parsed.permissionContext = item.value(QStringLiteral("permissionContext")).toMap();
    parsed.parameters = item.value(QStringLiteral("parameters")).toMap();
    return parsed;
}

bool NavigationItemModel::itemVisible(const Item &item) const
{
    if (!item.visible)
        return false;
    if (item.permissionId.isEmpty())
        return true;
    if (!m_permissionEvaluator || !m_permissionEvaluator->hasProvider())
        return false;
    return m_permissionEvaluator->isVisible(item.permissionId, item.permissionContext);
}

bool NavigationItemModel::itemEnabled(const Item &item) const
{
    if (!item.enabled)
        return false;
    if (item.permissionId.isEmpty())
        return true;
    if (!m_permissionEvaluator || !m_permissionEvaluator->hasProvider())
        return false;
    return m_permissionEvaluator->isEnabled(item.permissionId, item.permissionContext);
}

QVariantMap NavigationItemModel::itemToMap(const Item &item, int row) const
{
    return {
        {QStringLiteral("index"), row},
        {QStringLiteral("route"), item.route},
        {QStringLiteral("label"), item.label},
        {QStringLiteral("icon"), item.icon},
        {QStringLiteral("section"), sectionName(item.section)},
        {QStringLiteral("group"), item.group},
        {QStringLiteral("badge"), item.badge},
        {QStringLiteral("visible"), itemVisible(item)},
        {QStringLiteral("enabled"), itemEnabled(item)},
        {QStringLiteral("selected"), item.route == m_selectedRoute},
        {QStringLiteral("permissionId"), item.permissionId},
        {QStringLiteral("permissionContext"), item.permissionContext},
        {QStringLiteral("parameters"), item.parameters}
    };
}

void NavigationItemModel::refreshPermissionRoles()
{
    if (m_items.isEmpty())
        return;
    emit dataChanged(index(0, 0), index(m_items.size() - 1, 0), {VisibleRole, EnabledRole});
}

} // namespace LfQuick
