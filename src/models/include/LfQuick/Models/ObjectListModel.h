#pragma once

#include <LfQuick/Models/LfQuickModelsGlobal.h>

#include <QAbstractListModel>
#include <QMetaObject>
#include <QPointer>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKMODELS_EXPORT ObjectListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfObjectListModel)
    Q_PROPERTY(QList<QObject*> values READ values WRITE setValues NOTIFY valuesChanged FINAL)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    enum Role {
        ModelDataRole = Qt::UserRole
    };
    Q_ENUM(Role)

    explicit ObjectListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QList<QObject *> values() const;
    void setValues(const QList<QObject *> &values);

    Q_INVOKABLE QObject *get(int index) const;
    Q_INVOKABLE int indexOf(QObject *object) const;
    Q_INVOKABLE bool append(QObject *object);
    Q_INVOKABLE bool insert(int index, QObject *object);
    Q_INVOKABLE bool remove(QObject *object);
    Q_INVOKABLE bool removeAt(int index);
    Q_INVOKABLE bool move(int from, int to);
    Q_INVOKABLE void clear();

signals:
    void valuesChanged();
    void countChanged();
    void objectInserted(QObject *object, int index);
    void objectRemoved(QObject *object, int index);
    void objectMoved(QObject *object, int from, int to);

private:
    struct Entry {
        QPointer<QObject> object;
        QMetaObject::Connection destroyedConnection;
    };

    static QList<QObject *> normalize(const QList<QObject *> &values);
    bool insertInternal(int index, QObject *object, bool notifyValues);
    bool removeAtInternal(int index, bool notifyValues);
    void connectEntry(Entry &entry, QObject *object);

    QList<Entry> m_entries;
};

} // namespace LfQuick
