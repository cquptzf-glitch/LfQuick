#include <LfQuick/Ui/NavigationController.h>

namespace LfQuick {

NavigationController::NavigationController(QObject *parent)
    : QObject(parent)
{
}

void NavigationController::setModel(NavigationItemModel *model)
{
    if (m_model == model)
        return;
    m_model = model;
    if (m_model)
        m_model->setSelectedRoute(m_currentRoute);
    emit modelChanged();
}

bool NavigationController::navigate(const QString &route, const QVariantMap &parameters)
{
    const QString normalized = route.trimmed();
    if (normalized.isEmpty()) {
        emit navigationRejected(route, QStringLiteral("Route is empty"));
        return false;
    }

    if (m_model) {
        const int row = m_model->rowForRoute(normalized);
        if (row < 0) {
            emit navigationRejected(normalized, QStringLiteral("Route is not present in navigation model"));
            return false;
        }

        const QVariantMap item = m_model->get(row);
        if (!item.value(QStringLiteral("visible"), true).toBool()) {
            emit navigationRejected(normalized, QStringLiteral("Navigation item is hidden"));
            return false;
        }
        if (!item.value(QStringLiteral("enabled"), true).toBool()) {
            emit navigationRejected(normalized, QStringLiteral("Navigation item is disabled"));
            return false;
        }
    }

    emit navigateRequested(normalized, parameters);
    return true;
}

bool NavigationController::back()
{
    if (!m_canGoBack)
        return false;
    emit backRequested();
    return true;
}

bool NavigationController::forward()
{
    if (!m_canGoForward)
        return false;
    emit forwardRequested();
    return true;
}

void NavigationController::syncCurrentRoute(const QString &route, bool canGoBack)
{
    syncNavigationState(route, canGoBack, false);
}

void NavigationController::syncNavigationState(const QString &route,
                                               bool canGoBack,
                                               bool canGoForward)
{
    const QString normalized = route.trimmed();
    const bool changed = m_currentRoute != normalized
        || m_canGoBack != canGoBack
        || m_canGoForward != canGoForward;
    m_currentRoute = normalized;
    m_canGoBack = canGoBack;
    m_canGoForward = canGoForward;

    if (m_model)
        m_model->setSelectedRoute(m_currentRoute);

    if (changed)
        emit navigationStateChanged();
}

} // namespace LfQuick
