#include <LfQuick/Ui/QuickRuntime.h>

#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QuickPageProvider.h>

#include <QQmlEngine>
#include <QQuickItem>

namespace LfQuick {

QuickRuntime::QuickRuntime(QQmlEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_pages(std::make_unique<PageRegistry>())
    , m_providers(std::make_unique<PageProviderRegistry>())
    , m_errorCollector(std::make_unique<QmlErrorCollector>())
    , m_provider(std::make_shared<QuickPageProvider>(engine))
    , m_router(std::make_unique<PageRouter>(m_pages.get(), m_providers.get(), this))
{
    m_provider->setErrorCollector(m_errorCollector.get());
    m_providers->registerProvider(m_provider);

    connect(m_router.get(), &PageRouter::currentRouteChanged, this, [this] {
        emit currentRouteChanged();
        emit navigationChanged();
    });
    connect(m_router.get(), &PageRouter::historyChanged,
            this, &QuickRuntime::navigationChanged);
    connect(m_router.get(), &PageRouter::cachePolicyChanged,
            this, &QuickRuntime::cachePolicyChanged);
    connect(m_router.get(), &PageRouter::navigationFailed,
            this, &QuickRuntime::navigationFailed);
    connect(m_router.get(), &PageRouter::navigationBlocked,
            this, &QuickRuntime::navigationBlocked);
    connect(m_router.get(), &PageRouter::cacheEntryEvicted,
            this, &QuickRuntime::cacheEntryEvicted);
}

QuickRuntime::~QuickRuntime()
{
    m_router->reset();
    m_providers->unregisterProvider(QStringLiteral("quick"));
}

QString QuickRuntime::currentRoute() const
{
    return m_router ? m_router->currentRoute() : QString{};
}

bool QuickRuntime::canGoBack() const
{
    return m_router && m_router->canGoBack();
}

int QuickRuntime::depth() const
{
    return m_router ? m_router->depth() : 0;
}

QQuickItem *QuickRuntime::host() const
{
    return m_host;
}

void QuickRuntime::setHost(QQuickItem *host)
{
    if (m_host == host)
        return;

    m_host = host;
    m_provider->setHost(host);
    emit hostChanged();
}

QmlErrorCollector *QuickRuntime::errorCollector() const
{
    return m_errorCollector.get();
}

int QuickRuntime::maxRetainedLruPages() const
{
    return m_router ? m_router->maxRetainedLruPages() : 0;
}

void QuickRuntime::setMaxRetainedLruPages(int value)
{
    if (m_router)
        m_router->setMaxRetainedLruPages(value);
}

bool QuickRuntime::registerPage(const QString &route,
                                const QUrl &source,
                                int cachePolicy,
                                bool useErrorFallback)
{
    if (route.isEmpty() || !source.isValid() || source.isEmpty())
        return false;

    return m_pages->registerPage(
        descriptorFor(route, source, cachePolicy, useErrorFallback));
}

bool QuickRuntime::unregisterPage(const QString &route)
{
    return m_pages->unregisterPage(route);
}

bool QuickRuntime::push(const QString &route, const QVariantMap &parameters)
{
    if (!m_router || route.isEmpty())
        return false;

    if (m_router->currentRoute() == route)
        return true;

    RouteRequest request;
    request.route = route;
    request.parameters = parameters;
    return m_router->push(request).accepted;
}

bool QuickRuntime::replace(const QString &route, const QVariantMap &parameters)
{
    if (!m_router || route.isEmpty())
        return false;

    RouteRequest request;
    request.route = route;
    request.parameters = parameters;
    return m_router->replace(request).accepted;
}

bool QuickRuntime::back()
{
    return m_router && m_router->back().accepted;
}

void QuickRuntime::reset()
{
    if (m_router)
        m_router->reset();
}

PageDescriptor QuickRuntime::descriptorFor(const QString &route,
                                           const QUrl &source,
                                           int cachePolicy,
                                           bool useErrorFallback) const
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("quick");
    descriptor.source.insert(QStringLiteral("url"), source);
    descriptor.cachePolicy = static_cast<PageCachePolicy>(
        qBound(static_cast<int>(PageCachePolicy::None),
               cachePolicy,
               static_cast<int>(PageCachePolicy::Lru)));

    if (useErrorFallback) {
        descriptor.fallbackPolicy = FallbackPolicy::ErrorPage;
        descriptor.source.insert(
            QStringLiteral("fallbackUrl"),
            QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/PageLoadError.qml")));
    } else {
        descriptor.fallbackPolicy = FallbackPolicy::None;
    }

    return descriptor;
}

} // namespace LfQuick
