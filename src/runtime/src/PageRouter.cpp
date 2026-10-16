#include <LfQuick/Runtime/PageRouter.h>

#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>

#include <algorithm>

namespace LfQuick {
namespace {

QString providerError(const std::shared_ptr<IPageProvider> &provider,
                      const QString &fallbackMessage)
{
    if (provider) {
        const QString message = provider->lastError().trimmed();
        if (!message.isEmpty())
            return message;
    }
    return fallbackMessage;
}

QString creationFailureMessage(const QVariantMap &diagnostics)
{
    QStringList parts;
    const auto append = [&parts, &diagnostics](const QString &key, const QString &label) {
        const QString message = diagnostics.value(key).toString().trimmed();
        if (!message.isEmpty())
            parts.push_back(label + QStringLiteral(": ") + message);
    };

    append(QStringLiteral("primaryError"), QStringLiteral("Primary"));
    append(QStringLiteral("fallbackError"), QStringLiteral("Cross-provider fallback"));
    append(QStringLiteral("errorFallbackError"), QStringLiteral("Error fallback"));
    return parts.isEmpty() ? QStringLiteral("Page creation failed") : parts.join(QStringLiteral(" | "));
}

} // namespace

struct PageRouter::Entry
{
    PageDescriptor descriptor;
    QVariantMap parameters;
    std::unique_ptr<IPageInstance> instance;
    bool suspended = false;
    quint64 lastTouched = 0;
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

int PageRouter::maxRetainedLruPages() const noexcept
{
    return m_maxRetainedLruPages;
}

void PageRouter::setMaxRetainedLruPages(int value)
{
    value = std::max(0, value);
    if (m_maxRetainedLruPages == value)
        return;

    m_maxRetainedLruPages = value;
    enforceLruLimit();
    emit cachePolicyChanged();
}

bool PageRouter::addGuard(std::shared_ptr<INavigationGuard> guard)
{
    if (!guard)
        return false;

    const auto duplicate = std::find_if(m_guards.cbegin(), m_guards.cend(),
                                        [&guard](const auto &candidate) {
        return candidate.get() == guard.get();
    });
    if (duplicate != m_guards.cend())
        return false;

    m_guards.push_back(std::move(guard));
    return true;
}

void PageRouter::clearGuards()
{
    m_guards.clear();
}

RouteResult PageRouter::createEntry(const RouteRequest &request, Entry &entry)
{
    if (!m_pages || !m_providers) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("runtime"), 1,
                  QStringLiteral("Router dependencies are not configured")));
    }

    const auto descriptor = m_pages->find(request.route);
    if (!descriptor) {
        return RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 404,
                  QStringLiteral("Route is not registered")));
    }

    QVariantMap diagnostics;
    auto instance = createInstance(*descriptor, request.parameters, diagnostics);
    if (!instance) {
        diagnostics.insert(QStringLiteral("route"), request.route);
        return RouteResult::failure(
            request.route,
            Error(QStringLiteral("routing"), 3,
                  creationFailureMessage(diagnostics), diagnostics));
    }

    entry.descriptor = *descriptor;
    entry.parameters = request.parameters;
    entry.instance = std::move(instance);
    touch(entry);
    return RouteResult::success(request.route);
}

std::unique_ptr<IPageInstance> PageRouter::createInstance(const PageDescriptor &descriptor,
                                                           const QVariantMap &parameters,
                                                           QVariantMap &diagnostics)
{
    diagnostics.clear();
    diagnostics.insert(QStringLiteral("primaryProvider"), descriptor.provider);

    if (!m_providers) {
        diagnostics.insert(QStringLiteral("primaryError"),
                           QStringLiteral("Page provider registry is not configured"));
        return {};
    }

    const auto primaryProvider = m_providers->find(descriptor.provider);
    QString primaryError;

    if (!primaryProvider) {
        primaryError = QStringLiteral("Page provider '%1' is not registered")
                           .arg(descriptor.provider);
    } else if (!primaryProvider->canCreate(descriptor)) {
        primaryError = providerError(
            primaryProvider,
            QStringLiteral("Page provider '%1' rejected the descriptor")
                .arg(descriptor.provider));
    } else {
        auto primaryInstance = primaryProvider->create(descriptor, parameters);
        if (primaryInstance)
            return primaryInstance;

        primaryError = providerError(
            primaryProvider,
            QStringLiteral("Page provider '%1' failed to create the page")
                .arg(descriptor.provider));
    }

    diagnostics.insert(QStringLiteral("primaryError"), primaryError);

    const bool stateless = descriptor.metadata.value(QStringLiteral("stateless")).toBool();
    const bool crossProviderAllowed = descriptor.fallback.has_value()
        && (descriptor.fallbackPolicy == FallbackPolicy::LegacyBeforeActivate
            || (descriptor.fallbackPolicy == FallbackPolicy::LegacyIfStateless && stateless));

    if (crossProviderAllowed) {
        PageDescriptor fallbackDescriptor = descriptor;
        fallbackDescriptor.provider = descriptor.fallback->provider;
        fallbackDescriptor.source = descriptor.fallback->source;
        fallbackDescriptor.fallbackPolicy = FallbackPolicy::None;
        fallbackDescriptor.fallback.reset();

        diagnostics.insert(QStringLiteral("fallbackProvider"), fallbackDescriptor.provider);
        emit pageFallbackAttempted(descriptor.route,
                                   descriptor.provider,
                                   fallbackDescriptor.provider,
                                   primaryError);

        const auto fallbackProvider = m_providers->find(fallbackDescriptor.provider);
        QString fallbackError;
        if (!fallbackProvider) {
            fallbackError = QStringLiteral("Fallback provider '%1' is not registered")
                                .arg(fallbackDescriptor.provider);
        } else if (!fallbackProvider->canCreate(fallbackDescriptor)) {
            fallbackError = providerError(
                fallbackProvider,
                QStringLiteral("Fallback provider '%1' rejected the descriptor")
                    .arg(fallbackDescriptor.provider));
        } else {
            auto fallbackInstance = fallbackProvider->create(fallbackDescriptor, parameters);
            if (fallbackInstance) {
                diagnostics.insert(QStringLiteral("activeProvider"), fallbackDescriptor.provider);
                diagnostics.insert(QStringLiteral("fallbackStrategy"),
                                   QStringLiteral("cross-provider"));
                emit pageFallbackActivated(descriptor.route,
                                           fallbackDescriptor.provider,
                                           QStringLiteral("cross-provider"));
                return fallbackInstance;
            }

            fallbackError = providerError(
                fallbackProvider,
                QStringLiteral("Fallback provider '%1' failed to create the page")
                    .arg(fallbackDescriptor.provider));
        }

        diagnostics.insert(QStringLiteral("fallbackError"), fallbackError);
        emit pageFallbackFailed(descriptor.route, fallbackDescriptor.provider, fallbackError);
    }

    // ErrorPage is the final provider-native fallback. For the Legacy* policies
    // it runs only after the cross-provider attempt failed (or was unsafe).
    if (primaryProvider && descriptor.fallbackPolicy != FallbackPolicy::None) {
        QString reason = primaryError;
        const QString crossProviderError = diagnostics.value(QStringLiteral("fallbackError")).toString();
        if (!crossProviderError.isEmpty())
            reason += QStringLiteral("\nCross-provider fallback failed: ") + crossProviderError;

        auto errorInstance = primaryProvider->createErrorFallback(descriptor, parameters, reason);
        if (errorInstance) {
            diagnostics.insert(QStringLiteral("activeProvider"), descriptor.provider);
            diagnostics.insert(QStringLiteral("fallbackStrategy"), QStringLiteral("error-page"));
            emit pageFallbackActivated(descriptor.route,
                                       descriptor.provider,
                                       QStringLiteral("error-page"));
            return errorInstance;
        }

        const QString errorFallbackError = primaryProvider->lastError().trimmed();
        if (!errorFallbackError.isEmpty()) {
            diagnostics.insert(QStringLiteral("errorFallbackError"), errorFallbackError);
            emit pageFallbackFailed(descriptor.route, descriptor.provider, errorFallbackError);
        }
    }

    return {};
}

RouteResult PageRouter::push(const RouteRequest &request)
{
    RouteResult guardFailure;
    if (!passesGuards(NavigationAction::Push, request.route, request.parameters, guardFailure))
        return guardFailure;

    Entry entry;
    auto result = createEntry(request, entry);
    if (!result.accepted) {
        emit navigationFailed(request.route, result.error.message());
        return result;
    }

    deactivateCurrent(true);
    m_history.push_back(std::move(entry));
    if (!activate(m_history.back())) {
        auto failure = RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 6,
                  QStringLiteral("Failed to activate page")));
        emit navigationFailed(request.route, failure.error.message());
        destroy(m_history.back());
        m_history.pop_back();
        return failure;
    }

    enforceLruLimit();
    notifyRouteChange();
    return result;
}

RouteResult PageRouter::replace(const RouteRequest &request)
{
    RouteResult guardFailure;
    if (!passesGuards(NavigationAction::Replace, request.route, request.parameters, guardFailure))
        return guardFailure;

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
    if (!activate(m_history.back())) {
        auto failure = RouteResult::failure(request.route,
            Error(QStringLiteral("routing"), 6,
                  QStringLiteral("Failed to activate page")));
        emit navigationFailed(request.route, failure.error.message());
        destroy(m_history.back());
        m_history.pop_back();
        return failure;
    }

    enforceLruLimit();
    notifyRouteChange();
    return result;
}

RouteResult PageRouter::back()
{
    if (!canGoBack()) {
        const auto route = currentRoute();
        auto result = RouteResult::failure(route,
            Error(QStringLiteral("routing"), 4,
                  QStringLiteral("No back history")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    Entry &target = m_history[m_history.size() - 2];

    RouteResult guardFailure;
    if (!passesGuards(NavigationAction::Back, target.descriptor.route,
                      target.parameters, guardFailure)) {
        return guardFailure;
    }

    if (!ensureInstance(target)) {
        const auto route = target.descriptor.route;
        const auto result = RouteResult::failure(
            route,
            Error(QStringLiteral("routing"), 5,
                  QStringLiteral("Failed to recreate cached page")));
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
            Error(QStringLiteral("routing"), 6,
                  QStringLiteral("Failed to activate page")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    enforceLruLimit();
    notifyRouteChange();
    return RouteResult::success(currentRoute());
}

void PageRouter::reset()
{
    // Hard reset intentionally bypasses navigation guards. It is used for
    // shutdown/recovery and must always be able to release every page.
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

    QVariantMap diagnostics;
    entry.instance = createInstance(entry.descriptor, entry.parameters, diagnostics);
    if (!entry.instance)
        return false;

    entry.suspended = false;
    touch(entry);
    return true;
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
    touch(entry);
    return true;
}

void PageRouter::deactivateCurrent(bool suspend)
{
    if (m_history.empty() || !m_history.back().instance)
        return;

    Entry &entry = m_history.back();
    entry.instance->deactivate();

    if (!suspend)
        return;

    switch (entry.descriptor.cachePolicy) {
    case PageCachePolicy::None:
        destroy(entry);
        break;
    case PageCachePolicy::Singleton:
    case PageCachePolicy::KeepAlive:
    case PageCachePolicy::Lru:
        entry.instance->suspend();
        entry.suspended = true;
        touch(entry);
        break;
    }
}

void PageRouter::destroy(Entry &entry)
{
    if (!entry.instance)
        return;

    entry.instance->destroy();
    entry.instance.reset();
    entry.suspended = false;
}

void PageRouter::touch(Entry &entry)
{
    entry.lastTouched = ++m_touchSequence;
}

void PageRouter::enforceLruLimit()
{
    if (m_history.size() <= 1)
        return;

    while (true) {
        int retained = 0;
        Entry *oldest = nullptr;

        // The current page is never an eviction candidate. The limit applies
        // only to inactive LRU pages that are retained for fast back navigation.
        for (size_t i = 0; i + 1 < m_history.size(); ++i) {
            Entry &entry = m_history[i];
            if (entry.descriptor.cachePolicy != PageCachePolicy::Lru || !entry.instance)
                continue;

            ++retained;
            if (!oldest || entry.lastTouched < oldest->lastTouched)
                oldest = &entry;
        }

        if (retained <= m_maxRetainedLruPages || !oldest)
            break;

        const QString route = oldest->descriptor.route;
        destroy(*oldest);
        emit cacheEntryEvicted(route);
    }
}

bool PageRouter::passesGuards(NavigationAction action,
                              const QString &toRoute,
                              const QVariantMap &parameters,
                              RouteResult &failureResult)
{
    if (m_history.empty() || m_guards.empty())
        return true;

    NavigationContext context;
    context.action = action;
    context.fromRoute = currentRoute();
    context.toRoute = toRoute;
    context.parameters = parameters;

    for (const auto &guard : m_guards) {
        if (!guard)
            continue;

        const auto decision = guard->evaluate(context);
        if (decision.allowed)
            continue;

        const QString reason = decision.reason.isEmpty()
            ? QStringLiteral("Navigation blocked by guard")
            : decision.reason;

        failureResult = RouteResult::failure(
            toRoute,
            Error(QStringLiteral("navigation-guard"), 10, reason));
        emit navigationBlocked(context.fromRoute, context.toRoute, reason);
        emit navigationFailed(toRoute, reason);
        return false;
    }

    return true;
}

void PageRouter::notifyRouteChange()
{
    emit currentRouteChanged(currentRoute());
    emit historyChanged();
}

} // namespace LfQuick
