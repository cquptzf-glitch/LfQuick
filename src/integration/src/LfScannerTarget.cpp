#include <LfQuick/Integration/LfScannerTarget.h>

#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/InputTarget.h>
#include <LfQuick/Ui/LfAttached.h>

#include <QtQml/qqml.h>

namespace LfQuick {

LfScannerTarget::LfScannerTarget(QObject *parent)
    : QObject(parent)
    , m_inputTarget(new InputTarget(this))
{
    connect(m_inputTarget, &InputTarget::inputReceived,
            this, &LfScannerTarget::inputReceived);
    syncInputTarget();
}

LfScannerTarget::~LfScannerTarget()
{
    unregisterFromRouter();
}

void LfScannerTarget::setRouter(InputRouter *router)
{
    if (m_router == router)
        return;

    unregisterFromRouter();
    QObject::disconnect(m_routerDestroyedConnection);
    m_routerDestroyedConnection = {};

    m_router = router;
    if (m_router) {
        m_routerDestroyedConnection = connect(m_router, &QObject::destroyed, this, [this] {
            m_router = nullptr;
            m_routerDestroyedConnection = {};
            emit routerChanged();
        });
    }
    registerWithRouter();
    emit routerChanged();
}

void LfScannerTarget::setTarget(QQuickItem *target)
{
    if (m_target == target)
        return;

    QObject::disconnect(m_acceptScannerConnection);
    QObject::disconnect(m_focusConnection);
    QObject::disconnect(m_targetDestroyedConnection);
    m_acceptScannerConnection = {};
    m_focusConnection = {};
    m_targetDestroyedConnection = {};

    m_target = target;
    reconnectTarget();
    syncInputTarget();
    emit targetChanged();
    emit stateChanged();
}

void LfScannerTarget::setTargetId(const QString &targetId)
{
    const QString normalized = targetId.trimmed();
    if (m_targetId == normalized)
        return;
    m_targetId = normalized;
    syncInputTarget();
    emit targetIdChanged();
}

void LfScannerTarget::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    syncInputTarget();
    emit stateChanged();
}

void LfScannerTarget::setFollowActiveFocus(bool follow)
{
    if (m_followActiveFocus == follow)
        return;
    m_followActiveFocus = follow;
    syncInputTarget();
    emit stateChanged();
}

bool LfScannerTarget::active() const noexcept
{
    if (m_followActiveFocus)
        return m_target && m_target->hasActiveFocus();
    return m_manualActive;
}

void LfScannerTarget::setActive(bool active)
{
    if (m_manualActive == active)
        return;
    m_manualActive = active;
    syncInputTarget();
    emit stateChanged();
}

bool LfScannerTarget::acceptScanner() const noexcept
{
    return m_attached && m_attached->acceptScanner();
}

void LfScannerTarget::setPriority(int priority)
{
    if (m_priority == priority)
        return;
    m_priority = priority;
    syncInputTarget();
    emit stateChanged();
}

void LfScannerTarget::unregisterFromRouter()
{
    if (m_router && m_inputTarget)
        m_router->unregisterTarget(m_inputTarget);
}

void LfScannerTarget::registerWithRouter()
{
    if (m_router && m_inputTarget)
        m_router->registerTarget(m_inputTarget);
}

void LfScannerTarget::reconnectTarget()
{
    m_attached = nullptr;
    if (!m_target)
        return;

    m_attached = qmlAttachedPropertiesObject<Lf>(m_target, true);
    if (m_attached) {
        m_acceptScannerConnection = connect(m_attached, &LfAttached::acceptScannerChanged,
                                            this, [this] {
            syncInputTarget();
            emit stateChanged();
        });
    }

    m_focusConnection = connect(m_target, &QQuickItem::activeFocusChanged,
                                this, [this] {
        if (!m_followActiveFocus)
            return;
        syncInputTarget();
        emit stateChanged();
    });
    m_targetDestroyedConnection = connect(m_target, &QObject::destroyed, this, [this] {
        m_target = nullptr;
        m_attached = nullptr;
        m_acceptScannerConnection = {};
        m_focusConnection = {};
        m_targetDestroyedConnection = {};
        syncInputTarget();
        emit targetChanged();
        emit stateChanged();
    });
}

void LfScannerTarget::syncInputTarget()
{
    if (!m_inputTarget)
        return;

    const QString effectiveId = !m_targetId.isEmpty()
        ? m_targetId
        : (m_target ? m_target->objectName() : QString{});
    m_inputTarget->setId(effectiveId);
    m_inputTarget->setTarget(m_target);
    m_inputTarget->setEnabled(m_enabled);
    m_inputTarget->setActive(active());
    m_inputTarget->setAcceptScanner(acceptScanner());
    m_inputTarget->setPriority(m_priority);
}

} // namespace LfQuick
