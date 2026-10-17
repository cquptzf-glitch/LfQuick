#include <LfQuick/Runtime/PageRouter.h>

#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/PageRegistry.h>

#include <algorithm>
#include <utility>

namespace LfQuick {
namespace {

PageProviderError providerErrorInfo(const std::shared_ptr<IPageProvider> &provider,
                                    const QString &fallbackCode,
                                    const QString &fallbackMessage,
                                    const QVariantMap &fallbackDetails = {})
{
    PageProviderError error = provider ? provider->lastErrorInfo() : PageProviderError{};
    if (error.code.trimmed().isEmpty())
        error.code = fallbackCode;
    if (error.message.trimmed().isEmpty())
        error.message = fallbackMessage;
    for (auto it = fallbackDetails.cbegin(); it != fallbackDetails.cend(); ++it) {
        if (!error.details.contains(it.key()))
            error.details.insert(it.key(), it.value());
    }
    return error;
}

void insertProviderFailure(QVariantMap &diagnostics,
                           const QString &legacyKey,
                           const QString &structuredKey,
                           const PageProviderError &error)
{
    diagnostics.insert(legacyKey, error.message);
    diagnostics.insert(structuredKey, error.toVariantMap());
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
    QVariantMap savedState;
    std::shared_ptr<IPageInstance> instance;
    bool suspended = false;
    quint64 lastTouched = 0;
    qint64 cacheWeight = 1;
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

bool PageRouter::canGoForward() const noexcept
{
    return !m_forwardHistory.empty();
}

int PageRouter::depth() const noexcept
{
    return static_cast<int>(m_history.size());
}

int PageRouter::forwardDepth() const noexcept
{
    return static_cast<int>(m_forwardHistory.size());
}

QStringList PageRouter::history() const
{
    QStringList routes;
    routes.reserve(depth());
    for (const auto &entry : m_history)
        routes.push_back(entry.descriptor.route);
    return routes;
}

QStringList PageRouter::forwardHistory() const
{
    QStringList routes;
    routes.reserve(forwardDepth());
    for (auto it = m_forwardHistory.rbegin(); it != m_forwardHistory.rend(); ++it)
        routes.push_back(it->descriptor.route);
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

void PageRouter::setMaxRetainedLruWeight(qint64 value)
{
    value = std::max<qint64>(0, value);
    if (m_maxRetainedLruWeight == value)
        return;

    m_maxRetainedLruWeight = value;
    enforceLruLimit();
    emit cachePolicyChanged();
}

qint64 PageRouter::retainedLruWeight() const noexcept
{
    qint64 weight = 0;
    if (m_history.size() <= 1)
        return weight;

    for (size_t i = 0; i + 1 < m_history.size(); ++i) {
        const Entry &entry = m_history[i];
        if (entry.descriptor.cachePolicy == PageCachePolicy::Lru && entry.instance)
            weight += entry.cacheWeight;
    }
    return weight;
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

    entry.descriptor = *descriptor;
    entry.parameters = request.parameters;
    entry.cacheWeight = std::max<qint64>(1, descriptor->cacheWeight);

    if (descriptor->cachePolicy == PageCachePolicy::Singleton) {
        auto cached = m_singletons.find(descriptor->route);
        if (cached != m_singletons.end() && cached.value()) {
            if (!cached.value()->isValid()) {
                invalidateSingleton(descriptor->route, cached.value());
            } else {
                if (!request.parameters.isEmpty()
                    && !cached.value()->updateParameters(request.parameters)) {
                    QVariantMap details{
                        {QStringLiteral("route"), request.route},
                        {QStringLiteral("cachePolicy"), QStringLiteral("singleton")},
                        {QStringLiteral("parameters"), request.parameters}
                    };
                    return RouteResult::failure(
                        request.route,
                        Error(QStringLiteral("routing"), 7,
                              QStringLiteral("Singleton page rejected updated navigation parameters"),
                              details));
                }

                entry.instance = cached.value();
                entry.suspended = true;
                touch(entry);
                emit singletonReused(descriptor->route);
                return RouteResult::success(request.route);
            }
        }
    }

    QVariantMap diagnostics;
    auto created = createInstance(*descriptor, request.parameters, diagnostics);
    if (!created) {
        diagnostics.insert(QStringLiteral("route"), request.route);
        return RouteResult::failure(
            request.route,
            Error(QStringLiteral("routing"), 3,
                  creationFailureMessage(diagnostics), diagnostics));
    }

    // Every new instance receives one state-finalization callback. Stateful
    // providers use the empty map to complete their initial-load lifecycle;
    // technology-neutral providers inherit the no-op default implementation.
    if (!created->restoreState({})) {
        created->destroy();
        return RouteResult::failure(
            request.route,
            Error(QStringLiteral("routing"), 9,
                  QStringLiteral("Page rejected initial state finalization")));
    }

    entry.instance = std::shared_ptr<IPageInstance>(std::move(created));
    touch(entry);

    if (descriptor->cachePolicy == PageCachePolicy::Singleton) {
        m_singletons.insert(descriptor->route, entry.instance);
        emit singletonCreated(descriptor->route);
        emit cacheStateChanged();
    }

    return RouteResult::success(request.route);
}

std::unique_ptr<IPageInstance> PageRouter::createInstance(const PageDescriptor &descriptor,
                                                           const QVariantMap &parameters,
                                                           QVariantMap &diagnostics)
{
    diagnostics.clear();

    if (descriptor.featureSwitch == FeatureSwitch::Legacy) {
        diagnostics.insert(QStringLiteral("featureSwitch"), QStringLiteral("legacy"));
        if (!descriptor.fallback.has_value()) {
            const PageProviderError error{
                QStringLiteral("runtime.legacy-provider-missing"),
                QStringLiteral("Legacy feature switch requires a fallback provider"),
                {{QStringLiteral("route"), descriptor.route}}
            };
            insertProviderFailure(diagnostics, QStringLiteral("primaryError"),
                                  QStringLiteral("primaryFailure"), error);
            return {};
        }

        PageDescriptor legacyDescriptor = descriptor;
        legacyDescriptor.provider = descriptor.fallback->provider;
        legacyDescriptor.source = descriptor.fallback->source;
        legacyDescriptor.featureSwitch = FeatureSwitch::Quick;
        legacyDescriptor.fallbackPolicy = FallbackPolicy::None;
        legacyDescriptor.fallback.reset();

        auto instance = createInstance(legacyDescriptor, parameters, diagnostics);
        diagnostics.insert(QStringLiteral("featureSwitch"), QStringLiteral("legacy"));
        diagnostics.insert(QStringLiteral("selectedProvider"), legacyDescriptor.provider);
        return instance;
    }

    diagnostics.insert(QStringLiteral("featureSwitch"),
                       descriptor.featureSwitch == FeatureSwitch::Quick
                           ? QStringLiteral("quick")
                           : QStringLiteral("auto"));
    diagnostics.insert(QStringLiteral("primaryProvider"), descriptor.provider);

    if (!m_providers) {
        const PageProviderError error{
            QStringLiteral("runtime.provider-registry-missing"),
            QStringLiteral("Page provider registry is not configured"),
            {{QStringLiteral("route"), descriptor.route}}
        };
        insertProviderFailure(diagnostics, QStringLiteral("primaryError"),
                              QStringLiteral("primaryFailure"), error);
        return {};
    }

    const auto primaryProvider = m_providers->find(descriptor.provider);
    PageProviderError primaryFailure;

    if (!primaryProvider) {
        primaryFailure = {
            QStringLiteral("runtime.provider-not-registered"),
            QStringLiteral("Page provider '%1' is not registered").arg(descriptor.provider),
            {{QStringLiteral("provider"), descriptor.provider},
             {QStringLiteral("route"), descriptor.route}}
        };
    } else if (!primaryProvider->canCreate(descriptor)) {
        primaryFailure = providerErrorInfo(
            primaryProvider,
            QStringLiteral("runtime.provider-rejected"),
            QStringLiteral("Page provider '%1' rejected the descriptor")
                .arg(descriptor.provider),
            {{QStringLiteral("provider"), descriptor.provider},
             {QStringLiteral("route"), descriptor.route},
             {QStringLiteral("stage"), QStringLiteral("canCreate")}});
    } else {
        auto primaryInstance = primaryProvider->create(descriptor, parameters);
        if (primaryInstance)
            return primaryInstance;

        primaryFailure = providerErrorInfo(
            primaryProvider,
            QStringLiteral("runtime.provider-create-failed"),
            QStringLiteral("Page provider '%1' failed to create the page")
                .arg(descriptor.provider),
            {{QStringLiteral("provider"), descriptor.provider},
             {QStringLiteral("route"), descriptor.route},
             {QStringLiteral("stage"), QStringLiteral("create")}});
    }

    insertProviderFailure(diagnostics, QStringLiteral("primaryError"),
                          QStringLiteral("primaryFailure"), primaryFailure);

    const bool stateless = descriptor.metadata.value(QStringLiteral("stateless")).toBool();
    const bool crossProviderAllowed = descriptor.featureSwitch != FeatureSwitch::Quick
        && descriptor.fallback.has_value()
        && (descriptor.fallbackPolicy == FallbackPolicy::LegacyBeforeActivate
            || (descriptor.fallbackPolicy == FallbackPolicy::LegacyIfStateless && stateless));

    if (crossProviderAllowed) {
        PageDescriptor fallbackDescriptor = descriptor;
        fallbackDescriptor.provider = descriptor.fallback->provider;
        fallbackDescriptor.source = descriptor.fallback->source;
        fallbackDescriptor.featureSwitch = FeatureSwitch::Quick;
        fallbackDescriptor.fallbackPolicy = FallbackPolicy::None;
        fallbackDescriptor.fallback.reset();

        diagnostics.insert(QStringLiteral("fallbackProvider"), fallbackDescriptor.provider);
        emit pageFallbackAttempted(descriptor.route,
                                   descriptor.provider,
                                   fallbackDescriptor.provider,
                                   primaryFailure.message);

        const auto fallbackProvider = m_providers->find(fallbackDescriptor.provider);
        PageProviderError fallbackFailure;
        if (!fallbackProvider) {
            fallbackFailure = {
                QStringLiteral("runtime.fallback-provider-not-registered"),
                QStringLiteral("Fallback provider '%1' is not registered")
                    .arg(fallbackDescriptor.provider),
                {{QStringLiteral("provider"), fallbackDescriptor.provider},
                 {QStringLiteral("route"), descriptor.route}}
            };
        } else if (!fallbackProvider->canCreate(fallbackDescriptor)) {
            fallbackFailure = providerErrorInfo(
                fallbackProvider,
                QStringLiteral("runtime.fallback-provider-rejected"),
                QStringLiteral("Fallback provider '%1' rejected the descriptor")
                    .arg(fallbackDescriptor.provider),
                {{QStringLiteral("provider"), fallbackDescriptor.provider},
                 {QStringLiteral("route"), descriptor.route},
                 {QStringLiteral("stage"), QStringLiteral("canCreate")}});
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

            fallbackFailure = providerErrorInfo(
                fallbackProvider,
                QStringLiteral("runtime.fallback-provider-create-failed"),
                QStringLiteral("Fallback provider '%1' failed to create the page")
                    .arg(fallbackDescriptor.provider),
                {{QStringLiteral("provider"), fallbackDescriptor.provider},
                 {QStringLiteral("route"), descriptor.route},
                 {QStringLiteral("stage"), QStringLiteral("create")}});
        }

        insertProviderFailure(diagnostics, QStringLiteral("fallbackError"),
                              QStringLiteral("fallbackFailure"), fallbackFailure);
        emit pageFallbackFailed(descriptor.route, fallbackDescriptor.provider,
                                fallbackFailure.message);
    }

    if (primaryProvider && descriptor.fallbackPolicy != FallbackPolicy::None) {
        QString reason = primaryFailure.message;
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

        PageProviderError fallbackFailure = providerErrorInfo(
            primaryProvider,
            QStringLiteral("runtime.error-fallback-create-failed"),
            QStringLiteral("Provider-native error fallback failed"),
            {{QStringLiteral("provider"), descriptor.provider},
             {QStringLiteral("route"), descriptor.route},
             {QStringLiteral("stage"), QStringLiteral("error-fallback")}});
        if (!fallbackFailure.message.isEmpty()) {
            insertProviderFailure(diagnostics, QStringLiteral("errorFallbackError"),
                                  QStringLiteral("errorFallbackFailure"), fallbackFailure);
            emit pageFallbackFailed(descriptor.route, descriptor.provider,
                                    fallbackFailure.message);
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

    clearForwardHistory();
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

    clearForwardHistory();
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
    Entry leaving = std::move(m_history.back());
    destroy(leaving);
    m_history.pop_back();
    m_forwardHistory.push_back(std::move(leaving));

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

RouteResult PageRouter::forward()
{
    if (!canGoForward()) {
        const auto route = currentRoute();
        auto result = RouteResult::failure(route,
            Error(QStringLiteral("routing"), 8,
                  QStringLiteral("No forward history")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    Entry &target = m_forwardHistory.back();
    RouteResult guardFailure;
    if (!passesGuards(NavigationAction::Forward, target.descriptor.route,
                      target.parameters, guardFailure)) {
        return guardFailure;
    }

    if (!ensureInstance(target)) {
        const auto route = target.descriptor.route;
        const auto result = RouteResult::failure(
            route,
            Error(QStringLiteral("routing"), 5,
                  QStringLiteral("Failed to recreate forward page")));
        emit navigationFailed(route, result.error.message());
        return result;
    }

    deactivateCurrent(true);
    Entry next = std::move(m_forwardHistory.back());
    m_forwardHistory.pop_back();
    m_history.push_back(std::move(next));

    if (!activate(m_history.back())) {
        Entry failed = std::move(m_history.back());
        destroy(failed);
        m_history.pop_back();
        m_forwardHistory.push_back(std::move(failed));
        if (!m_history.empty())
            activate(m_history.back());

        const auto result = RouteResult::failure(
            m_forwardHistory.back().descriptor.route,
            Error(QStringLiteral("routing"), 6,
                  QStringLiteral("Failed to activate forward page")));
        emit navigationFailed(result.route, result.error.message());
        return result;
    }

    enforceLruLimit();
    notifyRouteChange();
    return RouteResult::success(currentRoute());
}

void PageRouter::reset()
{
    for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
        if (it == m_history.rbegin() && it->instance && it->instance->isValid())
            it->instance->deactivate();
        destroy(*it);
    }

    const bool hadEntries = !m_history.empty() || !m_forwardHistory.empty();
    m_history.clear();
    clearForwardHistory();
    destroySingletonCache();
    if (hadEntries)
        notifyRouteChange();
}

bool PageRouter::ensureInstance(Entry &entry)
{
    if (entry.instance && !entry.instance->isValid()) {
        if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton) {
            invalidateSingleton(entry.descriptor.route, entry.instance);
        } else {
            entry.instance->destroy();
            entry.instance.reset();
            entry.suspended = false;
        }
    }

    if (entry.instance)
        return true;

    if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton) {
        auto cached = m_singletons.find(entry.descriptor.route);
        if (cached != m_singletons.end() && cached.value()) {
            if (!cached.value()->isValid()) {
                invalidateSingleton(entry.descriptor.route, cached.value());
            } else {
                entry.instance = cached.value();
                entry.suspended = true;
                touch(entry);
                emit singletonReused(entry.descriptor.route);
                return true;
            }
        }
    }

    QVariantMap diagnostics;
    auto created = createInstance(entry.descriptor, entry.parameters, diagnostics);
    if (!created)
        return false;

    if (!created->restoreState(entry.savedState)) {
        created->destroy();
        return false;
    }

    entry.instance = std::shared_ptr<IPageInstance>(std::move(created));
    entry.suspended = false;
    touch(entry);

    if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton) {
        m_singletons.insert(entry.descriptor.route, entry.instance);
        emit singletonCreated(entry.descriptor.route);
        emit cacheStateChanged();
    }
    return true;
}

bool PageRouter::activate(Entry &entry)
{
    if (!ensureInstance(entry))
        return false;

    if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton
        && !entry.parameters.isEmpty()
        && !entry.instance->updateParameters(entry.parameters)) {
        return false;
    }

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
    if (!entry.instance->isValid()) {
        if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton)
            invalidateSingleton(entry.descriptor.route, entry.instance);
        else {
            entry.instance->destroy();
            entry.instance.reset();
            entry.suspended = false;
        }
        return;
    }

    entry.instance->deactivate();

    const bool singletonRetained = entry.descriptor.cachePolicy == PageCachePolicy::Singleton;
    if (!suspend && !singletonRetained)
        return;

    switch (entry.descriptor.cachePolicy) {
    case PageCachePolicy::None:
        if (suspend)
            destroy(entry);
        break;
    case PageCachePolicy::Singleton:
        entry.instance->suspend();
        entry.suspended = true;
        touch(entry);
        break;
    case PageCachePolicy::KeepAlive:
    case PageCachePolicy::Lru:
        if (suspend) {
            entry.instance->suspend();
            entry.suspended = true;
            touch(entry);
        }
        break;
    }
}

void PageRouter::destroy(Entry &entry)
{
    if (!entry.instance)
        return;

    if (entry.descriptor.cachePolicy == PageCachePolicy::Singleton) {
        entry.instance.reset();
        entry.suspended = false;
        return;
    }

    if (entry.instance->isValid())
        entry.savedState = entry.instance->saveState();

    entry.instance->destroy();
    entry.instance.reset();
    entry.suspended = false;
}

void PageRouter::invalidateSingleton(const QString &route,
                                     std::shared_ptr<IPageInstance> instance)
{
    if (!instance)
        return;

    bool removedFromCache = false;
    auto cached = m_singletons.find(route);
    if (cached != m_singletons.end() && cached.value() == instance) {
        m_singletons.erase(cached);
        removedFromCache = true;
    }

    const auto clearReferences = [&route, &instance](auto &entries) {
        for (auto &candidate : entries) {
            if (candidate.descriptor.cachePolicy != PageCachePolicy::Singleton
                || candidate.descriptor.route != route
                || candidate.instance != instance) {
                continue;
            }
            candidate.instance.reset();
            candidate.suspended = false;
        }
    };

    clearReferences(m_history);
    clearReferences(m_forwardHistory);
    instance->destroy();

    if (removedFromCache)
        emit cacheStateChanged();
}

void PageRouter::clearForwardHistory()
{
    if (m_forwardHistory.empty())
        return;
    for (auto &entry : m_forwardHistory)
        destroy(entry);
    m_forwardHistory.clear();
}

void PageRouter::destroySingletonCache()
{
    if (m_singletons.isEmpty())
        return;

    const auto instances = m_singletons.values();
    m_singletons.clear();
    for (const auto &instance : instances) {
        if (instance)
            instance->destroy();
    }
    emit cacheStateChanged();
}

void PageRouter::touch(Entry &entry)
{
    entry.lastTouched = ++m_touchSequence;
}

void PageRouter::enforceLruLimit()
{
    if (m_history.size() <= 1)
        return;

    bool evictedAny = false;
    while (true) {
        int retained = 0;
        qint64 retainedWeight = 0;
        Entry *oldest = nullptr;

        for (size_t i = 0; i + 1 < m_history.size(); ++i) {
            Entry &entry = m_history[i];
            if (entry.descriptor.cachePolicy != PageCachePolicy::Lru || !entry.instance)
                continue;

            ++retained;
            retainedWeight += entry.cacheWeight;
            if (!oldest || entry.lastTouched < oldest->lastTouched)
                oldest = &entry;
        }

        const bool countExceeded = retained > m_maxRetainedLruPages;
        const bool weightExceeded = m_maxRetainedLruWeight > 0
            && retainedWeight > m_maxRetainedLruWeight;
        if ((!countExceeded && !weightExceeded) || !oldest)
            break;

        const QString route = oldest->descriptor.route;
        destroy(*oldest);
        evictedAny = true;
        emit cacheEntryEvicted(route);
    }

    if (evictedAny)
        emit cacheStateChanged();
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
    emit cacheStateChanged();
}

} // namespace LfQuick