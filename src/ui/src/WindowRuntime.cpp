#include <LfQuick/Ui/WindowRuntime.h>

#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/WindowRouter.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QuickWindowProvider.h>

#include <QQmlEngine>
#include <QtQml/qqml.h>

#include <utility>

namespace LfQuick {
namespace {

QVariantMap toVariantMap(const WindowInfo &info)
{
    return {
        {QStringLiteral("windowId"), info.windowId},
        {QStringLiteral("instanceId"), info.instanceId},
        {QStringLiteral("ownerInstanceId"), info.ownerInstanceId},
        {QStringLiteral("state"), static_cast<int>(info.state)},
        {QStringLiteral("parameters"), info.parameters}
    };
}

} // namespace

WindowRuntime::WindowRuntime(QObject *parent)
    : QObject(parent)
    , m_errorCollector(std::make_unique<QmlErrorCollector>())
{
}

WindowRuntime::WindowRuntime(QQmlEngine *engine, QObject *parent)
    : WindowRuntime(parent)
{
    m_engine = engine;
    ensureInitialized();
}

WindowRuntime::~WindowRuntime()
{
    if (m_router)
        m_router->closeAll();
    if (m_providers && m_quickProvider)
        m_providers->unregisterProvider(m_quickProvider->id());
}

int WindowRuntime::count() const noexcept
{
    return m_router ? m_router->count() : 0;
}

QString WindowRuntime::activeInstanceId() const
{
    return m_router ? m_router->activeInstanceId() : QString{};
}

void WindowRuntime::setEngine(QQmlEngine *engine)
{
    if (m_engine == engine)
        return;
    if (m_router) {
        setLastError(QStringLiteral("QQmlEngine cannot change after WindowRuntime initialization"));
        return;
    }
    m_engine = engine;
}

bool WindowRuntime::registerProvider(std::shared_ptr<IWindowProvider> provider)
{
    if (!ensureInitialized() || !m_providers || !provider)
        return false;
    const bool accepted = m_providers->registerProvider(std::move(provider));
    if (!accepted)
        setLastError(QStringLiteral("Window provider registration was rejected"));
    else
        setLastError({});
    return accepted;
}

bool WindowRuntime::registerDescriptor(WindowDescriptor descriptor)
{
    if (!ensureInitialized() || !m_router)
        return false;
    const bool accepted = m_router->registerWindow(descriptor);
    if (!accepted)
        setLastError(QStringLiteral("Window descriptor registration was rejected"));
    else
        setLastError({});
    return accepted;
}

bool WindowRuntime::registerWindow(const QString &windowId,
                                   const QUrl &source,
                                   int openPolicy,
                                   int ownerClosePolicy,
                                   const QVariantMap &options)
{
    if (!ensureInitialized() || !m_router)
        return false;
    if (windowId.trimmed().isEmpty()) {
        setLastError(QStringLiteral("Window id is empty"));
        return false;
    }
    if (!source.isValid() || source.isEmpty()) {
        setLastError(QStringLiteral("Window source URL is empty or invalid"));
        return false;
    }
    if (!validOpenPolicy(openPolicy)) {
        setLastError(QStringLiteral("Invalid window open policy"));
        return false;
    }
    if (!validOwnerClosePolicy(ownerClosePolicy)) {
        setLastError(QStringLiteral("Invalid owner close policy"));
        return false;
    }

    WindowDescriptor descriptor;
    descriptor.id = windowId.trimmed();
    descriptor.provider = QStringLiteral("quick-window");
    descriptor.source = options;
    descriptor.source.insert(QStringLiteral("url"), source);
    descriptor.openPolicy = static_cast<WindowOpenPolicy>(openPolicy);
    descriptor.ownerClosePolicy = static_cast<OwnerClosePolicy>(ownerClosePolicy);

    const bool accepted = m_router->registerWindow(descriptor);
    if (!accepted)
        setLastError(QStringLiteral("Window is already registered or descriptor is invalid"));
    else
        setLastError({});
    return accepted;
}

bool WindowRuntime::unregisterWindow(const QString &windowId)
{
    if (!ensureInitialized() || !m_router)
        return false;
    const bool accepted = m_router->unregisterWindow(windowId);
    if (!accepted)
        setLastError(QStringLiteral("Window is not registered or still has live instances"));
    else
        setLastError({});
    return accepted;
}

QString WindowRuntime::open(const QString &windowId,
                            const QVariantMap &parameters,
                            const QString &ownerInstanceId,
                            const QVariantMap &restoreState,
                            const QVariantMap &metadata)
{
    if (!ensureInitialized() || !m_router)
        return {};

    WindowOpenRequest request;
    request.windowId = windowId;
    request.parameters = parameters;
    request.ownerInstanceId = ownerInstanceId;
    request.restoreState = restoreState;
    request.metadata = metadata;

    const WindowResult result = m_router->open(request);
    if (!acceptResult(result))
        return {};
    return result.instanceId;
}

bool WindowRuntime::activate(const QString &instanceId)
{
    return ensureInitialized() && m_router
        && acceptResult(m_router->activate(instanceId));
}

bool WindowRuntime::reparent(const QString &instanceId,
                             const QString &ownerInstanceId)
{
    return ensureInitialized() && m_router
        && acceptResult(m_router->reparent(instanceId, ownerInstanceId));
}

bool WindowRuntime::reload(const QString &instanceId)
{
    return ensureInitialized() && m_router
        && acceptResult(m_router->reloadInstance(instanceId));
}

bool WindowRuntime::close(const QString &instanceId)
{
    return ensureInitialized() && m_router
        && acceptResult(m_router->close(instanceId));
}

bool WindowRuntime::closeAll()
{
    return ensureInitialized() && m_router
        && acceptResult(m_router->closeAll());
}

QVariantMap WindowRuntime::saveState(const QString &instanceId) const
{
    if (!m_router)
        return {};
    const auto state = m_router->saveState(instanceId);
    return state ? *state : QVariantMap{};
}

QVariantMap WindowRuntime::windowInfo(const QString &instanceId) const
{
    if (!m_router)
        return {};
    const auto info = m_router->find(instanceId);
    return info ? toVariantMap(*info) : QVariantMap{};
}

QVariantList WindowRuntime::instances(const QString &windowId) const
{
    QVariantList result;
    if (!m_router)
        return result;

    const QList<WindowInfo> infos = m_router->instances(windowId);
    result.reserve(infos.size());
    for (const WindowInfo &info : infos)
        result.push_back(toVariantMap(info));
    return result;
}

void WindowRuntime::classBegin()
{
}

void WindowRuntime::componentComplete()
{
    ensureInitialized();
}

bool WindowRuntime::ensureInitialized()
{
    if (m_router)
        return true;

    if (!m_engine)
        m_engine = qmlEngine(this);
    if (!m_engine) {
        setLastError(QStringLiteral("WindowRuntime requires a QQmlEngine"));
        return false;
    }

    m_providers = std::make_unique<WindowProviderRegistry>();
    m_quickProvider = std::make_shared<QuickWindowProvider>(m_engine);
    m_quickProvider->setErrorCollector(m_errorCollector.get());
    if (!m_providers->registerProvider(m_quickProvider)) {
        m_quickProvider.reset();
        m_providers.reset();
        setLastError(QStringLiteral("Failed to register the default Quick window provider"));
        return false;
    }

    m_router = std::make_unique<WindowRouter>(m_providers.get(), this);

    connect(m_router.get(), &WindowRouter::windowsChanged,
            this, &WindowRuntime::windowsChanged);
    connect(m_router.get(), &WindowRouter::activeWindowChanged,
            this, &WindowRuntime::activeWindowChanged);
    connect(m_router.get(), &WindowRouter::windowOpened,
            this, &WindowRuntime::windowOpened);
    connect(m_router.get(), &WindowRouter::windowActivated,
            this, &WindowRuntime::windowActivated);
    connect(m_router.get(), &WindowRouter::windowOwnerChanged,
            this, &WindowRuntime::windowOwnerChanged);
    connect(m_router.get(), &WindowRouter::windowClosed,
            this, &WindowRuntime::windowClosed);
    connect(m_router.get(), &WindowRouter::windowReloaded,
            this, &WindowRuntime::windowReloaded);
    connect(m_router.get(), &WindowRouter::windowOpenFailed,
            this, [this](const QString &windowId, const QString &message) {
        setLastError(message);
        emit windowOpenFailed(windowId, message);
    });
    connect(m_router.get(), &WindowRouter::windowReloadFailed,
            this, [this](const QString &windowId,
                         const QString &instanceId,
                         const QString &message) {
        setLastError(message);
        emit windowReloadFailed(windowId, instanceId, message);
    });
    connect(m_router.get(), &WindowRouter::windowCloseBlocked,
            this, [this](const QString &windowId,
                         const QString &instanceId,
                         const QString &reason) {
        setLastError(reason.isEmpty()
                         ? QStringLiteral("Window close was blocked")
                         : reason);
        emit windowCloseBlocked(windowId, instanceId, reason);
    });

    setLastError({});
    return true;
}

bool WindowRuntime::validOpenPolicy(int policy) const noexcept
{
    return policy == static_cast<int>(WindowOpenPolicy::SingleInstance)
        || policy == static_cast<int>(WindowOpenPolicy::MultipleInstances);
}

bool WindowRuntime::validOwnerClosePolicy(int policy) const noexcept
{
    return policy == static_cast<int>(OwnerClosePolicy::Detach)
        || policy == static_cast<int>(OwnerClosePolicy::CloseWithOwner);
}

bool WindowRuntime::acceptResult(const WindowResult &result)
{
    if (!result.accepted) {
        setLastError(result.error.message());
        return false;
    }
    setLastError({});
    return true;
}

void WindowRuntime::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

} // namespace LfQuick
