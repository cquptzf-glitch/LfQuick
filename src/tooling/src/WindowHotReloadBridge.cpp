#include <LfQuick/Tooling/WindowHotReloadBridge.h>

#include <LfQuick/Runtime/WindowRouter.h>
#include <LfQuick/Ui/QuickWindowProvider.h>

#include <QSet>

namespace LfQuick {

WindowHotReloadBridge::WindowHotReloadBridge(QObject *parent)
    : QObject(parent)
{
}

WindowHotReloadBridge::~WindowHotReloadBridge()
{
    releaseControllerWatches(m_controller);
    if (m_runtime && m_runtime->quickProvider())
        m_runtime->quickProvider()->setSourceResolver({});
}

void WindowHotReloadBridge::setRuntime(WindowRuntime *runtime)
{
    if (m_runtime == runtime)
        return;

    clearWatches();
    QObject::disconnect(m_runtimeDestroyedConnection);
    m_runtimeDestroyedConnection = {};

    if (m_runtime && m_runtime->quickProvider())
        m_runtime->quickProvider()->setSourceResolver({});

    m_runtime = runtime;
    if (m_runtime) {
        m_runtimeDestroyedConnection = connect(m_runtime, &QObject::destroyed, this, [this] {
            m_runtime = nullptr;
            m_runtimeDestroyedConnection = {};
            m_watches.clear();
            emit watchedWindowsChanged();
            emit runtimeChanged();
        });
    }

    installSourceResolver();
    setLastError({});
    emit runtimeChanged();
}

void WindowHotReloadBridge::setController(QmlHotReloadController *controller)
{
    if (m_controller == controller)
        return;

    QmlHotReloadController *previous = m_controller.data();
    releaseControllerWatches(previous);
    QObject::disconnect(m_reloadConnection);
    QObject::disconnect(m_controllerDestroyedConnection);
    m_reloadConnection = {};
    m_controllerDestroyedConnection = {};

    m_controller = controller;
    if (m_controller) {
        if (!m_controller->engine() && m_runtime && m_runtime->engine())
            m_controller->setEngine(m_runtime->engine());

        m_reloadConnection = connect(
            m_controller, &QmlHotReloadController::reloadRequested,
            this, &WindowHotReloadBridge::handleReloadRequested);
        m_controllerDestroyedConnection = connect(
            m_controller, &QObject::destroyed, this, [this] {
                m_controller = nullptr;
                m_reloadConnection = {};
                m_controllerDestroyedConnection = {};
                installSourceResolver();
                emit controllerChanged();
            });
        acquireControllerWatches();
    } else {
        setLastError({});
    }

    installSourceResolver();
    emit controllerChanged();
}

void WindowHotReloadBridge::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    installSourceResolver();
    emit enabledChanged();
}

bool WindowHotReloadBridge::watchWindow(const QString &windowId,
                                        const QUrl &developmentSource)
{
    const QString id = windowId.trimmed();
    if (id.isEmpty()) {
        setLastError(QStringLiteral("Window id is empty"));
        return false;
    }
    if (!m_runtime || !m_runtime->router()) {
        setLastError(QStringLiteral("Window hot reload requires a WindowRuntime"));
        return false;
    }
    if (!m_controller) {
        setLastError(QStringLiteral("Window hot reload requires a QmlHotReloadController"));
        return false;
    }

    const QUrl logicalSource = logicalSourceForWindow(id);
    if (!logicalSource.isValid() || logicalSource.isEmpty()) {
        setLastError(QStringLiteral("Window '%1' has no valid Quick QML source").arg(id));
        return false;
    }

    const auto existing = m_watches.constFind(id);
    if (existing != m_watches.cend()
        && existing->logicalSource == logicalSource
        && existing->developmentSource == developmentSource) {
        setLastError({});
        return true;
    }

    Watch previous;
    const bool hadPrevious = existing != m_watches.cend();
    if (hadPrevious)
        previous = existing.value();

    // Remapping the same logical source requires releasing this bridge's old
    // reference first. If the new mapping fails, best-effort restore the old
    // watch so callers never silently lose hot reload ownership.
    if (hadPrevious)
        m_controller->unwatchSource(previous.logicalSource);

    if (!m_controller->watchSource(logicalSource, developmentSource)) {
        if (hadPrevious)
            m_controller->watchSource(previous.logicalSource, previous.developmentSource);
        setLastError(m_controller->lastError());
        return false;
    }

    m_watches.insert(id, Watch{logicalSource, developmentSource});
    installSourceResolver();
    setLastError({});
    if (!hadPrevious)
        emit watchedWindowsChanged();
    return true;
}

bool WindowHotReloadBridge::unwatchWindow(const QString &windowId)
{
    const QString id = windowId.trimmed();
    const auto it = m_watches.find(id);
    if (it == m_watches.end())
        return false;

    if (m_controller)
        m_controller->unwatchSource(it->logicalSource);
    m_watches.erase(it);
    installSourceResolver();
    setLastError({});
    emit watchedWindowsChanged();
    return true;
}

void WindowHotReloadBridge::clearWatches()
{
    if (m_watches.isEmpty()) {
        installSourceResolver();
        return;
    }

    releaseControllerWatches(m_controller);
    m_watches.clear();
    installSourceResolver();
    setLastError({});
    emit watchedWindowsChanged();
}

void WindowHotReloadBridge::installSourceResolver()
{
    if (!m_runtime || !m_runtime->quickProvider())
        return;

    // The controller may be shared by page hosts and other tooling. Restrict
    // the provider resolver to logical sources explicitly owned by this bridge
    // so unrelated hot-reload mappings cannot affect window creation/reload.
    // Capturing values rather than `this` also keeps the provider callback safe
    // across bridge destruction/reconfiguration.
    QSet<QString> watchedLogicalSources;
    if (m_enabled && m_controller) {
        for (auto it = m_watches.cbegin(); it != m_watches.cend(); ++it) {
            watchedLogicalSources.insert(
                it->logicalSource.toString(QUrl::FullyEncoded));
        }
    }

    const QPointer<QmlHotReloadController> controller = m_controller;
    m_runtime->quickProvider()->setSourceResolver(
        [controller, watchedLogicalSources = std::move(watchedLogicalSources)](
            const QUrl &logicalSource) {
            const QString key = logicalSource.toString(QUrl::FullyEncoded);
            if (!controller || !watchedLogicalSources.contains(key))
                return logicalSource;
            return controller->resolveSource(logicalSource);
        });
}

void WindowHotReloadBridge::releaseControllerWatches(
    QmlHotReloadController *controller)
{
    if (!controller)
        return;
    for (auto it = m_watches.cbegin(); it != m_watches.cend(); ++it)
        controller->unwatchSource(it->logicalSource);
}

void WindowHotReloadBridge::acquireControllerWatches()
{
    if (!m_controller)
        return;

    QStringList failures;
    for (auto it = m_watches.cbegin(); it != m_watches.cend(); ++it) {
        if (!m_controller->watchSource(it->logicalSource, it->developmentSource))
            failures.push_back(it.key());
    }
    if (!failures.isEmpty()) {
        setLastError(QStringLiteral("Failed to reacquire hot reload watches: %1")
                         .arg(failures.join(QStringLiteral(", "))));
    } else {
        setLastError({});
    }
}

void WindowHotReloadBridge::handleReloadRequested(
    const QStringList &logicalSources,
    qint64 revision)
{
    if (!m_enabled || !m_runtime || !m_runtime->router())
        return;

    QSet<QString> changed;
    for (const QString &source : logicalSources)
        changed.insert(source);

    int attempted = 0;
    int failed = 0;

    for (auto watch = m_watches.cbegin(); watch != m_watches.cend(); ++watch) {
        const QString key = watch->logicalSource.toString(QUrl::FullyEncoded);
        if (!changed.contains(key))
            continue;

        const QList<WindowInfo> instances = m_runtime->router()->instances(watch.key());
        for (const WindowInfo &instance : instances) {
            ++attempted;
            if (!m_runtime->reload(instance.instanceId))
                ++failed;
        }
    }

    if (failed > 0) {
        setLastError(QStringLiteral("%1 of %2 window reloads failed")
                         .arg(failed)
                         .arg(attempted));
    } else {
        setLastError({});
    }
    emit reloadBatchFinished(revision, attempted, failed);
}

QUrl WindowHotReloadBridge::logicalSourceForWindow(const QString &windowId) const
{
    if (!m_runtime || !m_runtime->router())
        return {};

    const auto descriptor = m_runtime->router()->descriptor(windowId);
    if (!descriptor || descriptor->provider != QStringLiteral("quick-window"))
        return {};

    const QVariant raw = descriptor->source.value(QStringLiteral("url"));
    if (raw.canConvert<QUrl>())
        return raw.toUrl();
    return QUrl(raw.toString());
}

void WindowHotReloadBridge::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

} // namespace LfQuick
