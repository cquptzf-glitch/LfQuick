#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>

namespace LfQuick {

class LFQUICKUI_EXPORT NavigationGuardState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dirty READ dirty WRITE setDirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(QString reason READ reason WRITE setReason NOTIFY reasonChanged FINAL)
    Q_PROPERTY(bool confirmRequired READ confirmRequired WRITE setConfirmRequired NOTIFY confirmRequiredChanged FINAL)

public:
    explicit NavigationGuardState(QObject *parent = nullptr);

    bool dirty() const noexcept { return m_dirty; }
    bool busy() const noexcept { return m_busy; }
    bool enabled() const noexcept { return m_enabled; }
    const QString &reason() const noexcept { return m_reason; }
    bool confirmRequired() const noexcept { return m_confirmRequired; }
    bool shouldBlock() const noexcept { return m_enabled && (m_dirty || m_busy); }

    void setDirty(bool value);
    void setBusy(bool value);
    void setEnabled(bool value);
    void setReason(QString value);
    void setConfirmRequired(bool value);

signals:
    void dirtyChanged();
    void busyChanged();
    void enabledChanged();
    void reasonChanged();
    void confirmRequiredChanged();
    void blockingStateChanged();

private:
    bool m_dirty = false;
    bool m_busy = false;
    bool m_enabled = true;
    QString m_reason;
    bool m_confirmRequired = true;
};

} // namespace LfQuick
