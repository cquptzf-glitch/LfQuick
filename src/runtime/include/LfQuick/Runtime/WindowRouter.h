#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>
#include <LfQuick/Runtime/WindowDescriptor.h>

#include <QHash>
#include <QObject>
#include <QStringList>

#include <memory>
#include <optional>

namespace LfQuick {

class IWindowInstance;
class WindowProviderRegistry;

struct LFQUICKRUNTIME_EXPORT WindowInfo
{
    WindowId windowId;
    WindowInstanceId instanceId;
    WindowInstanceId ownerInstanceId;
    WindowState state = WindowState::Created;
    QVariantMap parameters;
};

class LFQUICKRUNTIME_EXPORT WindowRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY windowsChanged FINAL)
    Q_PROPERTY(QString activeInstanceId READ activeInstanceId NOTIFY activeWindowChanged FINAL)

public:
    // `providers` is a non-owning dependency. It may be null, in which case
    // open() returns structured provider-registry diagnostics, but a non-null
    // registry must outlive this router.
    explicit WindowRouter(WindowProviderRegistry *providers, QObject *parent = nullptr);
    ~WindowRouter() override;

    bool registerWindow(const WindowDescriptor &descriptor);
    bool unregisterWindow(QStringView windowId);
    std::optional<WindowDescriptor> descriptor(QStringView windowId) const;

    WindowResult open(const WindowOpenRequest &request);
    WindowResult activate(QStringView instanceId);
    WindowResult reparent(QStringView instanceId, QStringView ownerInstanceId = {});
    WindowResult close(QStringView instanceId);
    WindowResult closeAll();

    std::optional<WindowInfo> find(QStringView instanceId) const;
    QList<WindowInfo> instances(QStringView windowId = {}) const;
    std::optional<QVariantMap> saveState(QStringView instanceId) const;
    int count() const noexcept { return m_instances.size(); }
    QString activeInstanceId() const { return m_activeInstanceId; }

signals:
    void windowsChanged();
    void activeWindowChanged(const QString &instanceId);
    void windowOpened(const QString &windowId, const QString &instanceId);
    void windowActivated(const QString &windowId, const QString &instanceId);
    void windowOwnerChanged(const QString &windowId,
                            const QString &instanceId,
                            const QString &ownerInstanceId);
    void windowClosed(const QString &windowId, const QString &instanceId);
    void windowOpenFailed(const QString &windowId, const QString &message);
    void windowCloseBlocked(const QString &windowId,
                            const QString &instanceId,
                            const QString &reason);

private:
    struct Record;

    std::shared_ptr<Record> findRecord(QStringView instanceId) const;
    std::shared_ptr<Record> findSingleInstance(QStringView windowId) const;
    QList<std::shared_ptr<Record>> closeClosure(const std::shared_ptr<Record> &root) const;
    WindowResult closeRecords(const QList<std::shared_ptr<Record>> &records,
                              const std::shared_ptr<Record> &root);
    bool wouldFormOwnerCycle(QStringView instanceId, QStringView ownerInstanceId) const;
    void setActive(const std::shared_ptr<Record> &record);
    void clearActiveIf(QStringView instanceId);

    WindowProviderRegistry *m_providers = nullptr;
    QHash<WindowId, WindowDescriptor> m_descriptors;
    QHash<WindowInstanceId, std::shared_ptr<Record>> m_instances;
    QString m_activeInstanceId;
};

} // namespace LfQuick
