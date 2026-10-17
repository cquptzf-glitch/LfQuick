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

bool QuickRuntime::canGoForward() const
{
    return m_router && m_router->canGoForward();
}

int QuickRuntime::depth() const
{
    return m_router ? m_router->depth() : 0;
}

int QuickRuntime::forwardDepth() const
{
    return m_router ? m_router->forwardDepth() : 0;
}

QQuickItem *QuickRuntime::host() const
{
    return m_host;
}

void QuickRuntime::setHost(QQuickItem *host)
{
    if (m_host == host)
        return;

    // The host is the scene root for routed Quick pages. Keeping history or
    // caches alive across a host switch would leave instances attached to the
    // previous scene tree while currentRoute still reports them as active.
    if (m_router)
        m_router->reset();

    QObject::disconnect(m_hostDestroyedConnection);
    m_hostDestroyedConnection = {};
    m_host = host;
    m_provider->setHost(host);

    if (m_host) {
        m_hostDestroyedConnection = connect(m_host, &QObject::destroyed, this, [this] {
            m_host = nullptr;
            m_hostDestroyedConnection = {};
            if (m_provider)
                m_provider->setHost(nullptr);
            if (m_router)
                m_router->reset();
            emit hostChanged();
        });
    }

    emit hostChanged();
}

QmlErrorCollector *QuickRuntime::errorCollector() const
{
    return m_errorCollector.get();
}

NavigationGuardController *QuickRuntime::navigationGuard() const
{
    return m_navigationGuardController.get();
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

qint64 QuickRuntime::maxRetainedLruWeight() const
{
    return m_router ? m_router->maxRetainedLruWeight() : 0;
}

void QuickRuntime::setMaxRetainedLruWeight(qint64 value)
{
    if (m_router)
        m_router->setMaxRetainedLruWeight(value);
}

qint64 QuickRuntime::retainedLruWeight() const
{
    return m_router ? m_router->retainedLruWeight() : 0;
}

int QuickRuntime::singletonCount() const
{
    return m_router ? m_router->singletonCount() : 0;
}

bool QuickRuntime::registerProvider(std::shared_ptr<IPageProvider> provider)
{
    return m_providers && m_providers->registerProvider(std::move(provider));
}

bool QuickRuntime::registerDescriptor(PageDescriptor descriptor)
{
    return m_pages && m_pages->registerPage(std::move(descriptor));
}

bool QuickRuntime::addNavigationGuard(std::shared_ptr<INavigationGuard> guard)
{
    return m_router && m_router->addGuard(std::move(guard));
}

void QuickRuntime::clearNavigationGuards()
{
    if (!m_router)
        return;
    m_router->clearGuards();
    if (m_navigationGuardController)
        m_router->addGuard(m_navigationGuardController->guardHandle());
}

void QuickRuntime::setQuickActivationCallback(std::function<void()> callback)
{
    m_provider->setActivationCallback(std::move(callback));
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
    return m_router->push(route, parameters).accepted;
}

bool QuickRuntime::replace(const QString &route, const QVariantMap &parameters)
{
    if (!m_router)
        return false;
    return m_router->replace(route, parameters).accepted;
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
