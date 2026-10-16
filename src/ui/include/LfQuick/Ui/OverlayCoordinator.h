#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QHash>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class OperationHandle;

class LFQUICKUI_EXPORT OverlayCoordinator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged FINAL)
    Q_PROPERTY(int busyCount READ busyCount NOTIFY busyChanged FINAL)
    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyChanged FINAL)
    Q_PROPERTY(double busyProgress READ busyProgress NOTIFY busyChanged FINAL)
    Q_PROPERTY(bool busyCancelable READ busyCancelable NOTIFY busyChanged FINAL)

public:
    explicit OverlayCoordinator(QObject *parent = nullptr);

    bool busy() const;
    int busyCount() const;
    QString busyMessage() const;
    double busyProgress() const;
    bool busyCancelable() const;

    Q_INVOKABLE QString beginBusy(const QString &message = {}, bool cancelable = false);
    Q_INVOKABLE bool endBusy(const QString &token);
    Q_INVOKABLE QString bindOperation(QObject *operation);
    Q_INVOKABLE void cancelTopBusy();

signals:
    void busyChanged();
    void busyCancelRequested(const QString &token);

private:
    struct Entry {
        QString message;
        bool cancelable = false;
        double progress = 0.0;
        QPointer<OperationHandle> operation;
        QMetaObject::Connection progressConnection;
        QMetaObject::Connection messageConnection;
        QMetaObject::Connection cancelableConnection;
        QMetaObject::Connection stateConnection;
    };

    QString topToken() const;
    void removeEntry(const QString &token);

    QHash<QString, Entry> m_entries;
    QStringList m_order;
};

} // namespace LfQuick
