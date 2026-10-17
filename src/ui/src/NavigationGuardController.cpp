#include <LfQuick/Ui/NavigationGuardController.h>

#include <LfQuick/Core/Route.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <QPointer>

#include <algorithm>

namespace LfQuick {
namespace {

class ControllerGuard final : public INavigationGuard
{
public:
    explicit ControllerGuard(NavigationGuardController *controller)
        : m_controller(controller) {}

    NavigationDecision evaluate(const NavigationContext &context) override
    {
        return m_controller ? m_controller->evaluate(context)
                            : NavigationDecision::allow();
    }

private:
    QPointer<NavigationGuardController> m_controller;
};

} // namespace

NavigationGuardController::NavigationGuardController(PageRouter *router, QObject *parent)
    : QObject(parent)
    , m_router(router)
    , m_guard(std::make_shared<ControllerGuard>(this))
{
    connect(&m_state, &NavigationGuardState::blockingStateChanged,
            this, [this] {
        if (!m_state.shouldBlock())
            clearPendingRequests();
    });

    if (m_router) {
        connect(m_router, &QObject::destroyed, this, [this] {
            m_router = nullptr;
            clearPendingRequests();
        });
    }
}

NavigationGuardController::~NavigationGuardController() = default;

QString NavigationGuardController::pendingFromRoute() const
{
    return m_pending ? m_pending->fromRoute : QString{};
}

QString NavigationGuardController::pendingRoute() const
{
    return m_pending ? m_pending->toRoute : QString{};
}

int NavigationGuardController::pendingAction() const noexcept
{
    return m_pending ? static_cast<int>(m_pending->action) : -1;
}

QVariantMap NavigationGuardController::pendingParameters() const
{
    return m_pending ? m_pending->parameters : QVariantMap{};
}

void NavigationGuardController::setPendingPolicy(PendingPolicy policy)
{
    if (m_pendingPolicy == policy)
        return;

    m_pendingPolicy = policy;
    if (m_pendingPolicy != PendingPolicy::Queue && !m_queue.isEmpty())
        dropQueuedRequests(QStringLiteral("Pending navigation policy changed"));
    emit pendingPolicyChanged();
}

void NavigationGuardController::setMaxQueuedRequests(int value)
{
    value = std::clamp(value, 0, 64);
    if (m_maxQueuedRequests == value)
        return;

    m_maxQueuedRequests = value;
    while (m_queue.size() > m_maxQueuedRequests) {
        const NavigationContext dropped = m_queue.takeLast();
        emit navigationRequestDropped(dropped.fromRoute, dropped.toRoute,
                                      QStringLiteral("Navigation guard queue limit reduced"));
    }
    emit pendingChanged();
    emit pendingPolicyChanged();
}

NavigationDecision NavigationGuardController::evaluate(const NavigationContext &context)
{
    if (m_bypassOnce && sameNavigation(*m_bypassOnce, context)) {
        m_bypassOnce.reset();
        return NavigationDecision::allow();
    }

    if (!m_state.shouldBlock())
        return NavigationDecision::allow();

    const QString reason = effectiveReason();

    if (!m_pending) {
        setPending(context);
        emitPendingPrompt();
        return NavigationDecision::deny(reason);
    }

    if (sameNavigation(*m_pending, context) || isQueued(context))
        return NavigationDecision::deny(reason);

    switch (m_pendingPolicy) {
    case PendingPolicy::KeepFirst:
        emit navigationRequestDropped(context.fromRoute, context.toRoute,
                                      QStringLiteral("A navigation confirmation is already pending"));
        break;
    case PendingPolicy::ReplaceLatest:
        setPending(context);
        emitPendingPrompt();
        break;
    case PendingPolicy::Queue:
        if (m_maxQueuedRequests <= 0 || m_queue.size() >= m_maxQueuedRequests) {
            emit navigationRequestDropped(context.fromRoute, context.toRoute,
                                          QStringLiteral("Navigation guard queue is full"));
        } else {
            m_queue.append(context);
            emit pendingChanged();
            emit navigationRequestQueued(context.fromRoute, context.toRoute, m_queue.size());
        }
        break;
    }

    return NavigationDecision::deny(reason);
}

void NavigationGuardController::cancelPending()
{
    m_bypassOnce.reset();
    clearPending();
    promoteNextQueued();
}

bool NavigationGuardController::confirmPending()
{
    if (!m_router || !m_pending || !m_state.confirmRequired() || m_retryInProgress)
        return false;

    const NavigationContext context = *m_pending;
    clearPending();
    m_bypassOnce = context;
    m_retryInProgress = true;

    RouteResult result;
    switch (context.action) {
    case NavigationAction::Push:
        result = m_router->push({context.toRoute, context.parameters, {}});
        break;
    case NavigationAction::Replace:
        result = m_router->replace({context.toRoute, context.parameters, {}});
        break;
    case NavigationAction::Back:
        result = m_router->back();
        break;
    }

    m_retryInProgress = false;
    m_bypassOnce.reset();

    if (result.accepted) {
        // Requests captured while the old page was blocked refer to the old
        // navigation origin. Replaying them after a successful route change
        // would be surprising, so they are explicitly discarded as stale.
        dropQueuedRequests(QStringLiteral("Navigation origin changed after confirmation"));
    } else {
        promoteNextQueued();
    }
    return result.accepted;
}

void NavigationGuardController::clearPendingRequests()
{
    m_bypassOnce.reset();
    const bool hadPending = m_pending.has_value() || !m_queue.isEmpty();
    m_pending.reset();
    m_queue.clear();
    if (hadPending)
        emit pendingChanged();
}

QString NavigationGuardController::effectiveReason() const
{
    if (!m_state.reason().trimmed().isEmpty())
        return m_state.reason().trimmed();
    if (m_state.busy() && m_state.dirty())
        return QStringLiteral("The current page is busy and has unsaved changes");
    if (m_state.busy())
        return QStringLiteral("The current operation is still running");
    return QStringLiteral("The current page has unsaved changes");
}

bool NavigationGuardController::sameNavigation(const NavigationContext &left,
                                                const NavigationContext &right) const
{
    return left.action == right.action
        && left.fromRoute == right.fromRoute
        && left.toRoute == right.toRoute
        && left.parameters == right.parameters;
}

bool NavigationGuardController::isQueued(const NavigationContext &context) const
{
    return std::any_of(m_queue.cbegin(), m_queue.cend(),
                       [this, &context](const NavigationContext &queued) {
        return sameNavigation(queued, context);
    });
}

void NavigationGuardController::setPending(const NavigationContext &context)
{
    m_pending = context;
    emit pendingChanged();
}

void NavigationGuardController::clearPending()
{
    if (!m_pending)
        return;
    m_pending.reset();
    emit pendingChanged();
}

void NavigationGuardController::emitPendingPrompt()
{
    if (!m_pending)
        return;

    const QString reason = effectiveReason();
    if (m_state.confirmRequired()) {
        emit confirmationRequested(m_pending->fromRoute, m_pending->toRoute, reason);
    } else {
        emit hardBlocked(m_pending->fromRoute, m_pending->toRoute, reason);
    }
}

void NavigationGuardController::promoteNextQueued()
{
    if (m_pending || m_queue.isEmpty() || !m_state.shouldBlock())
        return;

    while (!m_queue.isEmpty()) {
        const NavigationContext next = m_queue.takeFirst();
        if (m_router && next.fromRoute != m_router->currentRoute()) {
            emit navigationRequestDropped(next.fromRoute, next.toRoute,
                                          QStringLiteral("Queued navigation origin is stale"));
            continue;
        }

        setPending(next);
        emitPendingPrompt();
        return;
    }
    emit pendingChanged();
}

void NavigationGuardController::dropQueuedRequests(const QString &reason)
{
    if (m_queue.isEmpty())
        return;

    const QList<NavigationContext> queued = m_queue;
    m_queue.clear();
    for (const NavigationContext &context : queued)
        emit navigationRequestDropped(context.fromRoute, context.toRoute, reason);
    emit pendingChanged();
}

} // namespace LfQuick
