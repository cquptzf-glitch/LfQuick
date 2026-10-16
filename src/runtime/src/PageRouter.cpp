#include <LfQuick/Runtime/PageRouter.h>

#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>

namespace LfQuick {

struct PageRouter::Entry
{
    PageDescriptor descriptor;
    QVariantMap parameters;
    std::unique_ptr<IPageInstance> instance;
    bool suspended = false;
};

PageRouter::PageRouter(PageRegistry *pages,
                       PageProviderRegistry *providers,
                       QObject *parent)
    : QObject(parent)
    , m_pages(pages)
    , m_providers(providers)
{
}

PageRouter::~PageRouter()
{
    reset();
}

QString PageRouter::currentRoute() const
{
    return m_history.empty() ? QString{} : m_history.back().descriptor.route;
}

bool PageRouter::canGoBack() const noexcept
{
    return m_history.size() > 1;
}

int PageRouter::depth() const noexcept
{
    return static_cast<int>(m_history.size());
}

QStringList PageRouter::history() const
{
    QStringList routes;
    routes.reserve(depth());
    for (const auto &entry : m_history)
        routes.push_back(entry.descriptor.route);
    return routes;
}

RouteResult PageRouter::createEntry(const RouteRequest &request, Entry &entry)
{
    if (!m_pages || !m_providers) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("runtime"), 1, QStringLiteral("Router dependencies are not configured")));
    }

    const auto descriptor = m_pages->find(request.route);
    if (!descriptor) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 404, QStringLiteral("Route is not registered")));
    }

    const auto provider = m_providers->find(descriptor->provider);
    if (!provider || !provider->canCreate(*descriptor)) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 2, QStringLiteral("No compatible page provider")));
    }

    auto instance = provider->create(*descriptor, request.parameters);
    if (!instance) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 3, QStringLiteral("Page creation failed")));
    }

    entry.descriptor = *descriptor;
    entry.parameters = request.parameters;
    entry.instance = std::move(instance);
    return RouteResult::success(request.route);
}

RouteResult PageRouter::push(const RouteRequest &request)
{
    Entry entry;
    auto result = createEntry(request, entry);
    if (!result.accepted) {
        emit navigationFailed(request.route, result.error.message());
        return result;
    }

    deactivateCurrent(true);
    m_history.push_back(std::move(entry));
    activate(m_history.back());
    notifyRouteChange();
    return result;
}

RouteResult PageRouter::replace(const RouteRequest &request)
{
    Entry entry;
    auto result = createEntry(request, entry);
    if (!result.accepted) {
        emit navigationFailed(request.route, result.error.message());
        return result;
    }

    if (!m_history.empty()) {
        deactivateCurrent(false);
        destroy(m_history.back());
        m_history.pop_back();
    }

    m_history.push_back(std::move(entry));
    activate(m_history.back());
    notifyRouteChange();
    return result;
}

RouteResult PageRouter::back()
{
    if (!canGoBack()) {
        const auto route = currentRoute();
        auto result = RouteResult::failure(route,
            Error(QStringLiteral("routing"), 4, QStringLiteral("No back history")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    Entry &target = m_history[m_history.size() - 2];
    if (!ensureInstance(target)) {
        const auto route = target.descriptor.route;
        const auto result = RouteResult::failure(
            route,
            Error(QStringLiteral("routing"), 5, QStringLiteral("Failed to recreate cached page")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    deactivateCurrent(false);
    destroy(m_history.back());
    m_history.pop_back();
    if (!activate(m_history.back())) {
        const auto route = currentRoute();
        const auto result = RouteResult::failure(
            route,
            Error(QStringLiteral("routing"), 6, QStringLiteral("Failed to activate page")));
        emit navigationFailed(route, result.error.message());
        return result;
    }
    notifyRouteChange();
    return RouteResult::success(currentRoute());
}

void PageRouter::reset()
{
    for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
        if (it == m_history.rbegin() && it->instance)
            it->instance->deactivate();
        destroy(*it);
    }
    const bool hadEntries = !m_history.empty();
    m_history.clear();
    if (hadEntries)
        notifyRouteChange();
}

bool PageRouter::ensureInstance(Entry &entry)
{
    if (entry.instance)
        return true;

    if (!m_providers)
        return false;

    const auto provider = m_providers->find(entry.descriptor.provider);
    if (!provider || !provider->canCreate(entry.descriptor))
        return false;

    entry.instance = provider->create(entry.descriptor, entry.parameters);
    if (!entry.instance)
        return false;

    entry.suspended = false;
    return true;
}

void PageRouter::deactivateCurrent(bool suspend)
{
    if (m_history.empty() || !m_history.back().instance)
        return;

    m_history.back().instance->deactivate();

    // None means the page instance is not retained in the navigation history.
    // Singleton/KeepAlive/Lru currently share the retained-instance behavior;
    // a dedicated cache manager will implement true LRU eviction later.
    if (suspend && m_history.back().descriptor.cachePolicy != PageCachePolicy::None) {
        m_history.back().instance->suspend();
        m_history.back().suspended = true;
    }
}

bool PageRouter::activate(Entry &entry)
{
    if (!ensureInstance(entry))
        return false;

    if (entry.suspended) {
        entry.instance->resume();
        entry.suspended = false;
    }
    entry.instance->activate();
    return true;
}

void PageRouter::destroy(Entry &entry)
{
    if (!entry.instance)
        return;
    entry.instance->destroy();
    entry.instance.reset();
}

void PageRouter::notifyRouteChange()
{
    emit currentRouteChanged(currentRoute());
    emit historyChanged();
}

} // namespace LfQuick
