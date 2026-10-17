#include <LfQuick/Runtime/RuntimeTelemetryObserver.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <utility>

namespace LfQuick {

RuntimeTelemetryObserver::RuntimeTelemetryObserver(QObject *parent)
    : QObject(parent)
{
}

void RuntimeTelemetryObserver::setRouter(PageRouter *router)
{
    if (m_router == router)
        return;
    for (const auto &connection : std::as_const(m_connections))
        QObject::disconnect(connection);
    m_connections.clear();
    m_router = router;
    reconnect();
}

void RuntimeTelemetryObserver::setSink(std::shared_ptr<ILogSink> sink)
{
    m_sink = std::move(sink);
}

void RuntimeTelemetryObserver::reconnect()
{
    if (!m_router)
        return;

    m_connections.push_back(connect(m_router, &PageRouter::currentRouteChanged,
                                    this, [this](const QString &route) {
        write(QStringLiteral("runtime.route.changed"), TelemetryLevel::Info, {
            {QStringLiteral("route"), route},
            {QStringLiteral("depth"), m_router ? m_router->depth() : 0},
            {QStringLiteral("forwardDepth"), m_router ? m_router->forwardDepth() : 0}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::navigationFailed,
                                    this, [this](const QString &route, const QString &message) {
        write(QStringLiteral("runtime.route.failed"), TelemetryLevel::Error, {
            {QStringLiteral("route"), route},
            {QStringLiteral("error"), message}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::navigationBlocked,
                                    this, [this](const QString &fromRoute,
                                                 const QString &toRoute,
                                                 const QString &reason) {
        write(QStringLiteral("runtime.route.blocked"), TelemetryLevel::Warning, {
            {QStringLiteral("fromRoute"), fromRoute},
            {QStringLiteral("toRoute"), toRoute},
            {QStringLiteral("reason"), reason}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::pageFallbackAttempted,
                                    this, [this](const QString &route,
                                                 const QString &primaryProvider,
                                                 const QString &fallbackProvider,
                                                 const QString &primaryError) {
        write(QStringLiteral("runtime.fallback.attempted"), TelemetryLevel::Warning, {
            {QStringLiteral("route"), route},
            {QStringLiteral("primaryProvider"), primaryProvider},
            {QStringLiteral("fallbackProvider"), fallbackProvider},
            {QStringLiteral("reason"), primaryError}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::pageFallbackActivated,
                                    this, [this](const QString &route,
                                                 const QString &provider,
                                                 const QString &strategy) {
        write(QStringLiteral("runtime.fallback.activated"), TelemetryLevel::Info, {
            {QStringLiteral("route"), route},
            {QStringLiteral("provider"), provider},
            {QStringLiteral("strategy"), strategy}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::pageFallbackFailed,
                                    this, [this](const QString &route,
                                                 const QString &provider,
                                                 const QString &message) {
        write(QStringLiteral("runtime.fallback.failed"), TelemetryLevel::Error, {
            {QStringLiteral("route"), route},
            {QStringLiteral("provider"), provider},
            {QStringLiteral("error"), message}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::cacheEntryEvicted,
                                    this, [this](const QString &route) {
        write(QStringLiteral("runtime.cache.evicted"), TelemetryLevel::Debug, {
            {QStringLiteral("route"), route}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::singletonCreated,
                                    this, [this](const QString &route) {
        write(QStringLiteral("runtime.singleton.created"), TelemetryLevel::Debug, {
            {QStringLiteral("route"), route}
        });
    }));

    m_connections.push_back(connect(m_router, &PageRouter::singletonReused,
                                    this, [this](const QString &route) {
        write(QStringLiteral("runtime.singleton.reused"), TelemetryLevel::Debug, {
            {QStringLiteral("route"), route}
        });
    }));

    m_connections.push_back(connect(m_router, &QObject::destroyed, this, [this] {
        m_router = nullptr;
        m_connections.clear();
    }));
}

void RuntimeTelemetryObserver::write(QString name,
                                     TelemetryLevel level,
                                     QVariantMap attributes,
                                     qint64 durationMs) const
{
    if (!m_sink)
        return;
    TelemetryEvent event;
    event.name = std::move(name);
    event.category = QStringLiteral("runtime");
    event.level = level;
    event.durationMs = durationMs;
    event.attributes = std::move(attributes);
    m_sink->write(event);
}

} // namespace LfQuick
