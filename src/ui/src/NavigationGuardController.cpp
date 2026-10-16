#include <LfQuick/Ui/NavigationGuardController.h>

#include <LfQuick/Core/Route.h>
#include <LfQuick/Runtime/PageRouter.h>

#include <QPointer>

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
            clearPending();
    });
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

NavigationDecision NavigationGuardController::evaluate(const NavigationContext &context)
{
    if (m_bypassOnce && sameNavigation(*m_bypassOnce, context)) {
        m_bypassOnce.reset();
        return NavigationDecision::allow();
    }

    if (!m_state.shouldBlock())
        return NavigationDecision::allow();

    const QString reason = effectiveReason();
    setPending(context);

    if (m_state.confirmRequired()) {
        emit confirmationRequested(context.fromRoute, context.toRoute, reason);
    } else {
        emit hardBlocked(context.fromRoute, context.toRoute, reason);
    }

    return NavigationDecision::deny(reason);
}

void NavigationGuardController::cancelPending()
{
    m_bypassOnce.reset();
    clearPending();
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
    // If the router rejected before evaluating this guard (for example no back
    // history), make sure the bypass token cannot leak into a later request.
    m_bypassOnce.reset();
    return result.accepted;
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

} // namespace LfQuick
