#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QHash>
#include <QIdentityProxyModel>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT ModelRoleAdapter : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantMap roleMap READ roleMap WRITE setRoleMap NOTIFY roleMapChanged FINAL)

public:
    explicit ModelRoleAdapter(QObject *parent = nullptr);

    QVariantMap roleMap() const { return m_roleMap; }
    void setRoleMap(const QVariantMap &roleMap);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &proxyIndex,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &proxyIndex,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    Q_INVOKABLE int roleForName(const QString &name) const;
    Q_INVOKABLE QString sourceRoleName(const QString &alias) const;

signals:
    void roleMapChanged();

private:
    void rebuildRoleMappings();
    int resolveSourceRole(const QVariant &definition) const;

    QVariantMap m_roleMap;
    QHash<int, int> m_aliasToSourceRole;
    QHash<int, QByteArray> m_aliasRoleNames;
    QHash<QString, int> m_nameToRole;
};

} // namespace LfQuick
