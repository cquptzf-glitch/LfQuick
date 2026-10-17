#pragma once

#include <LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h>

#include <QAbstractListModel>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDIAGNOSTICS_EXPORT ObjectInspectorModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged FINAL)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        ValueRole,
        ValueTextRole,
        WritableRole,
        DynamicRole
    };
    Q_ENUM(Role)

    explicit ObjectInspectorModel(QObject *parent = nullptr);

    QObject *target() const noexcept { return m_target; }
    void setTarget(QObject *target);

    QString filter() const { return m_filter; }
    void setFilter(const QString &filter);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariant propertyValue(const QString &name) const;
    Q_INVOKABLE bool setPropertyValue(const QString &name, const QVariant &value);
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void targetChanged();
    void filterChanged();
    void countChanged();

private:
    struct Entry {
        QString name;
        QString type;
        QVariant value;
        bool writable = false;
        bool dynamic = false;
    };

    static QString valueText(const QVariant &value);
    void rebuild();

    QPointer<QObject> m_target;
    QString m_filter;
    QList<Entry> m_entries;
    QMetaObject::Connection m_destroyedConnection;
};

} // namespace LfQuick
