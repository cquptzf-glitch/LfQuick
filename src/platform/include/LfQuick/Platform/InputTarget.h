#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT InputTarget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged FINAL)
    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(bool acceptScanner READ acceptScanner WRITE setAcceptScanner NOTIFY acceptScannerChanged FINAL)
    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged FINAL)

public:
    explicit InputTarget(QObject *parent = nullptr);

    QString id() const { return m_id; }
    void setId(const QString &id);

    QObject *target() const noexcept { return m_target; }
    void setTarget(QObject *target);

    bool isEnabled() const noexcept { return m_enabled; }
    void setEnabled(bool enabled);

    bool isActive() const noexcept { return m_active; }
    void setActive(bool active);

    bool acceptScanner() const noexcept { return m_acceptScanner; }
    void setAcceptScanner(bool accept);

    int priority() const noexcept { return m_priority; }
    void setPriority(int priority);

    bool accepts(const QVariantMap &metadata) const;
    void deliver(const QString &value, const QVariantMap &metadata);

signals:
    void idChanged();
    void targetChanged();
    void enabledChanged();
    void activeChanged();
    void acceptScannerChanged();
    void priorityChanged();
    void inputReceived(const QString &value, const QVariantMap &metadata);

private:
    QString m_id;
    QPointer<QObject> m_target;
    bool m_enabled = true;
    bool m_active = false;
    bool m_acceptScanner = false;
    int m_priority = 0;
};

} // namespace LfQuick
