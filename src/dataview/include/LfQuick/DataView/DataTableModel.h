#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QAbstractTableModel>
#include <QHash>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT DataTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList columnKeys READ columnKeys WRITE setColumnKeys NOTIFY columnKeysChanged FINAL)
    Q_PROPERTY(QStringList columnTitles READ columnTitles WRITE setColumnTitles NOTIFY columnTitlesChanged FINAL)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    explicit DataTableModel(QObject *parent = nullptr);

    QStringList columnKeys() const;
    void setColumnKeys(const QStringList &keys);

    QStringList columnTitles() const;
    void setColumnTitles(const QStringList &titles);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void setRows(const QVariantList &rows);
    Q_INVOKABLE void appendRow(const QVariantMap &row);
    Q_INVOKABLE void insertRow(int row, const QVariantMap &value);
    Q_INVOKABLE void removeRow(int row);
    Q_INVOKABLE QVariantMap rowAt(int row) const;
    Q_INVOKABLE bool setCell(int row, const QString &key, const QVariant &value);
    Q_INVOKABLE void sortByKey(const QString &key, Qt::SortOrder order = Qt::AscendingOrder);

signals:
    void columnKeysChanged();
    void columnTitlesChanged();
    void countChanged();

private:
    int roleForKey(const QString &key) const;
    QString keyForRole(int role) const;
    void rebuildRoles();

    QStringList m_columnKeys;
    QStringList m_columnTitles;
    QVector<QVariantMap> m_rows;
    QHash<int, QByteArray> m_roles;
    QHash<QString, int> m_keyToRole;
};

} // namespace LfQuick
