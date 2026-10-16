#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QAbstractListModel>
#include <QPointer>
#include <QVariantList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class PermissionEvaluator;

enum class NavigationSection {
    Primary = 0,
    Secondary = 1,
    Footer = 2
};

class LFQUICKUI_EXPORT NavigationItemModel final : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString selectedRoute READ selectedRoute WRITE setSelectedRoute NOTIFY selectedRouteChanged FINAL)
    Q_PROPERTY(PermissionEvaluator* permissionEvaluator READ permissionEvaluator WRITE setPermissionEvaluator NOTIFY permissionEvaluatorChanged FINAL)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    enum Role {
        RouteRole = Qt::UserRole + 1,
        LabelRole,
        IconRole,
        SectionRole,
        GroupRole,
        BadgeRole,
        VisibleRole,
        EnabledRole,
        SelectedRole,
        PermissionIdRole,
        PermissionContextRole,
        ParametersRole
    };
    Q_ENUM(Role)

    explicit NavigationItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString selectedRoute() const { return m_selectedRoute; }
    void setSelectedRoute(const QString &route);

    PermissionEvaluator *permissionEvaluator() const { return m_permissionEvaluator; }
    void setPermissionEvaluator(PermissionEvaluator *evaluator);

    Q_INVOKABLE bool appendItem(const QVariantMap &item);
    Q_INVOKABLE void setItems(const QVariantList &items);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE int rowForRoute(const QString &route) const;

signals:
    void selectedRouteChanged();
    void permissionEvaluatorChanged();
    void countChanged();

private:
    struct Item {
        QString route;
        QString label;
        QString icon;
        NavigationSection section = NavigationSection::Primary;
        QString group;
        QVariant badge;
        bool visible = true;
        bool enabled = true;
        QString permissionId;
        QVariantMap permissionContext;
        QVariantMap parameters;
    };

    static NavigationSection sectionFromVariant(const QVariant &value);
    static QString sectionName(NavigationSection section);
    static std::optional<Item> itemFromMap(const QVariantMap &item);
    bool itemVisible(const Item &item) const;
    bool itemEnabled(const Item &item) const;
    QVariantMap itemToMap(const Item &item, int row) const;
    void refreshPermissionRoles();

    QList<Item> m_items;
    QString m_selectedRoute;
    QPointer<PermissionEvaluator> m_permissionEvaluator;
};

} // namespace LfQuick
