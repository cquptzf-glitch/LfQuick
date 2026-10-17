#pragma once

#include <LfQuick/Runtime/WindowDescriptor.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/QmlErrorCollector.h>

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QQmlParserStatus>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

class QQmlEngine;

namespace LfQuick {

class IWindowProvider;
class QuickWindowProvider;
class WindowProviderRegistry;
class WindowRouter;

// QML/C++ facade around the technology-neutral WindowRouter. The facade owns
// the default QuickWindowProvider, while callers can still register additional
// providers from C++ (for example WidgetWindowProvider in migration builds).
class LFQUICKUI_EXPORT WindowRuntime final : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int count READ count NOTIFY windowsChanged FINAL)
    Q_PROPERTY(QString activeInstanceId READ activeInstanceId NOTIFY activeWindowChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)
    Q_PROPERTY(QmlErrorCollector* errorCollector READ errorCollector CONSTANT FINAL)

public:
    enum class OpenPolicy {
        SingleInstance = static_cast<int>(WindowOpenPolicy::SingleInstance),
        MultipleInstances = static_cast<int>(WindowOpenPolicy::MultipleInstances)
    };
    Q_ENUM(OpenPolicy)

    enum class CloseWithOwnerPolicy {
        Detach = static_cast<int>(OwnerClosePolicy::Detach),
        CloseWithOwner = static_cast<int>(OwnerClosePolicy::CloseWithOwner)
    };
    Q_ENUM(CloseWithOwnerPolicy)

    explicit WindowRuntime(QObject *parent = nullptr);
    explicit WindowRuntime(QQmlEngine *engine, QObject *parent = nullptr);
    ~WindowRuntime() override;

    int count() const noexcept;
    QString activeInstanceId() const;
    QString lastError() const { return m_lastError; }
    QmlErrorCollector *errorCollector() const { return m_errorCollector.get(); }

    QQmlEngine *engine() const noexcept { return m_engine; }
    void setEngine(QQmlEngine *engine);

    WindowRouter *router() const noexcept { return m_router.get(); }
    bool registerProvider(std::shared_ptr<IWindowProvider> provider);
    bool registerDescriptor(WindowDescriptor descriptor);

    Q_INVOKABLE bool registerWindow(
        const QString &windowId,
        const QUrl &source,
        int openPolicy = static_cast<int>(WindowOpenPolicy::SingleInstance),
        int ownerClosePolicy = static_cast<int>(OwnerClosePolicy::CloseWithOwner),
        const QVariantMap &options = {});
    Q_INVOKABLE bool unregisterWindow(const QString &windowId);

    Q_INVOKABLE QString open(const QString &windowId,
                             const QVariantMap &parameters = {},
                             const QString &ownerInstanceId = {},
                             const QVariantMap &restoreState = {},
                             const QVariantMap &metadata = {});
    Q_INVOKABLE bool activate(const QString &instanceId);
    Q_INVOKABLE bool reparent(const QString &instanceId,
                              const QString &ownerInstanceId = {});
    Q_INVOKABLE bool reload(const QString &instanceId);
    Q_INVOKABLE bool close(const QString &instanceId);
    Q_INVOKABLE bool closeAll();

    Q_INVOKABLE QVariantMap saveState(const QString &instanceId) const;
    Q_INVOKABLE QVariantMap windowInfo(const QString &instanceId) const;
    Q_INVOKABLE QVariantList instances(const QString &windowId = {}) const;

    void classBegin() override;
    void componentComplete() override;

signals:
    void windowsChanged();
    void activeWindowChanged(const QString &instanceId);
    void lastErrorChanged();

    void windowOpened(const QString &windowId, const QString &instanceId);
    void windowActivated(const QString &windowId, const QString &instanceId);
    void windowOwnerChanged(const QString &windowId,
                            const QString &instanceId,
                            const QString &ownerInstanceId);
    void windowClosed(const QString &windowId, const QString &instanceId);
    void windowReloaded(const QString &windowId, const QString &instanceId);
    void windowOpenFailed(const QString &windowId, const QString &message);
    void windowReloadFailed(const QString &windowId,
                            const QString &instanceId,
                            const QString &message);
    void windowCloseBlocked(const QString &windowId,
                            const QString &instanceId,
                            const QString &reason);

private:
    bool ensureInitialized();
    bool validOpenPolicy(int policy) const noexcept;
    bool validOwnerClosePolicy(int policy) const noexcept;
    bool acceptResult(const WindowResult &result);
    void setLastError(const QString &error);

    QPointer<QQmlEngine> m_engine;
    std::unique_ptr<WindowProviderRegistry> m_providers;
    std::unique_ptr<QmlErrorCollector> m_errorCollector;
    std::shared_ptr<QuickWindowProvider> m_quickProvider;
    std::unique_ptr<WindowRouter> m_router;
    QString m_lastError;
};

} // namespace LfQuick
