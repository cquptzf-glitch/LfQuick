#include <LfQuick/Ui/QuickRuntime.h>

#include <LfQuick/Runtime/INavigationGuard.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>
#include <LfQuick/Runtime/PageRouter.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QuickPageProvider.h>

#include <QQmlEngine>
#include <QQuickItem>

#include <utility>

namespace LfQuick {

QuickRuntime::QuickRuntime(QQmlEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_pages(std::make_unique<PageRegistry>())
    , m_providers(std::make_unique<PageProviderRegistry>())
    , m_errorCollector(std::make_unique<QmlErrorCollector>())
    , m_provider(std::make_shared<QuickPageProvider>(engine))
    , m_router(std::make_unique<PageRouter>(m_pages.get(), m_providers.get(), this))
    , m_navigationGuardController(std::make_unique<NavigationGuardController>(m_router.get()))
{
    m_provider->setErrorCollector(m_errorCollector.get());
    m_providers->registerProvider(m_provider);
    m_router->addGuard(m_navigationGuardController->guardHandle());

    connect(m_router.get(), &PageRouter::currentRouteChanged, this, [this] {
        emit currentRouteChanged();
        emit navigationChanged();
    });
    connect(m_router.get(), &PageRouter::historyChanged,
            this, &QuickRuntime::navigationChanged);
    connect(m_router.get(), &PageRouter::cachePolicyChanged,
            this, &QuickRuntime::cachePolicyChanged);
    connect(m_router.get(), &PageRouter::cacheStateChanged,
            this, &QuickRuntime::cacheStateChanged);
    connect(m_router.get(), &PageRouter::navigationFailed,
            this, &QuickRuntime::navigationFailed);
    connect(m_router.get(), &PageRouter::navigationBlocked,
            this, &QuickRuntime::navigationBlocked);
    connect(m_router.get(), &PageRouter::cacheEntryEvicted,
            this, &QuickRuntime::cacheEntryEvicted);
    connect(m_router.get(), &PageRouter::pageFallbackAttempted,
            this, &QuickRuntime::pageFallbackAttempted);
    connect(m_router.get(), &PageRouter::pageFallbackActivated,
            this, &QuickRuntime::pageFallbackActivated);
    connect(m_router.get(), &PageRouter::pageFallbackFailed,
            this, &QuickRuntime::pageFallbackFailed);
}

QuickRuntime::~QuickRuntime()
{
    QObject::disconnect(m_hostDestroyedConnection);
    m_hostDestroyedConnection = {};
    m_router->reset();
}

QQmlEngine *QuickRuntime::engine() const noexcept
{
    return m_engine;
}

void QuickRuntime::setEngine(QQmlEngine *engine)
{
    if (m_engine == engine)
        return;
    m_engine = engine;
    m_provider->setEngine(engine);
    emit engineChanged();
}

QQuickItem *QuickRuntime::host() const noexcept
{
    return m_host;
}

void QuickRuntime::setHost(QQuickItem *host)
{
    if (m_host == host)
        return;

    QObject::disconnect(m_hostDestroyedConnection);
    m_hostDestroyedConnection = {};

    m_host = host;
    m_provider->setHost(host);
    if (m_host) {
        m_hostDestroyedConnection = connect(m_host, &QObject::destroyed, this, [this] {
            m_host = nullptr;
            m_provider->setHost(nullptr);
            emit hostChanged();
        });
    }
    emit hostChanged();
}

QmlErrorCollector *QuickRuntime::errorCollector() const noexcept
{
    return m_errorCollector.get();
}

QString QuickRuntime::currentRoute() const
{
    return m_router ? m_router->currentRoute() : QString{};
}

bool QuickRuntime::canGoBack() const noexcept
{
    return m_router && m_router->canGoBack();
}

bool QuickRuntime::canGoForward() const noexcept
{
    return m_router && m_router->canGoForward();
}

int QuickRuntime::depth() const noexcept
{
    return m_router ? m_router->depth() : 0;
}

int QuickRuntime::forwardDepth() const noexcept
{
    return m_router ? m_router->forwardDepth() : 0;
}

int QuickRuntime::maxRetainedLruPages() const noexcept
{
    return m_router ? m_router->maxRetainedLruPages() : 0;
}

void QuickRuntime::setMaxRetainedLruPages(int value)
{
    if (m_router)
        m_router->setMaxRetainedLruPages(value);
}

qint64 QuickRuntime::maxRetainedLruWeight() const noexcept
{
    return m_router ? m_router->maxRetainedLruWeight() : 0;
}

void QuickRuntime::setMaxRetainedLruWeight(qint64 value)
{
    if (m_router)
        m_router->setMaxRetainedLruWeight(value);
}

qint64 QuickRuntime::retainedLruWeight() const noexcept
{
    return m_router ? m_router->retainedLruWeight() : 0;
}

int QuickRuntime::singletonCount() const noexcept
{
    return m_router ? m_router->singletonCount() : 0;
}

NavigationGuardController *QuickRuntime::navigationGuardController() const noexcept
{
    return m_navigationGuardController.get();
}

QVariantMap QuickRuntime::lastError() const
{
    return m_provider ? m_provider->lastErrorInfo().toVariantMap() : QVariantMap{};
}

PageDescriptor QuickRuntime::descriptorFor(const QString &route,
                                           const QUrl &source,
                                           int cachePolicy,
                                           bool useErrorFallback,
                                           qint64 cacheWeight) const
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("quick");
    descriptor.source.insert(QStringLiteral("url"), source);
    descriptor.cachePolicy = static_cast<PageCachePolicy>(cachePolicy);
    descriptor.cacheWeight = cacheWeight;
    descriptor.fallbackPolicy = useErrorFallback ? FallbackPolicy::ErrorPage
                                                  : FallbackPolicy::None;
    return descriptor;
}

bool QuickRuntime::registerPage(const QString &route,
                                const QUrl &source,
                                int cachePolicy,
                                bool useErrorFallback,
                                qint64 cacheWeight)
{
    if (!m_pages)
        return false;
    return m_pages->registerPage(descriptorFor(route, source, cachePolicy,
                                               useErrorFallback, cacheWeight));
}

bool QuickRuntime::unregisterPage(const QString &route)
{
    if (!m_pages)
        return false;
    return m_pages->unregisterPage(route);
}

bool QuickRuntime::push(const QString &route, const QVariantMap &parameters)
{
    if (!m_router)
        return false;
    RouteRequest request;
    request.route = route;
    request.parameters = parameters;
    return m_router->push(request).accepted;
}

bool QuickRuntime::replace(const QString &route, const QVariantMap &parameters)
{
    if (!m_router)
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

bool QuickRuntime::forward()
{
    return m_router && m_router->forward().accepted;
}

void QuickRuntime::reset()
{
    if (m_router)
        m_router->reset();
}

} // namespace LfQuick
