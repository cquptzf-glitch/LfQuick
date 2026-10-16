#include <LfQuick/Ui/LfAttached.h>

namespace LfQuick {

PermissionBinding::PermissionBinding(QObject *parent)
    : QObject(parent)
{
}

void PermissionBinding::setEvaluator(PermissionEvaluator *evaluator)
{
    if (m_evaluator == evaluator)
        return;

    QObject::disconnect(m_providerConnection);
    QObject::disconnect(m_destroyedConnection);
    m_providerConnection = {};
    m_destroyedConnection = {};

    m_evaluator = evaluator;
    reconnectEvaluator();
    emit evaluatorChanged();
    emit stateChanged();
}

void PermissionBinding::setPermissionId(const QString &permissionId)
{
    const QString normalized = permissionId.trimmed();
    if (m_permissionId == normalized)
        return;

    m_permissionId = normalized;
    emit permissionIdChanged();
    emit stateChanged();
}

void PermissionBinding::setContext(const QVariantMap &context)
{
    if (m_context == context)
        return;

    m_context = context;
    emit contextChanged();
    emit stateChanged();
}

PermissionEvaluator::State PermissionBinding::state() const
{
    if (!active())
        return PermissionEvaluator::State::Allow;
    if (!m_evaluator)
        return PermissionEvaluator::State::Denied;
    return m_evaluator->evaluate(m_permissionId, m_context);
}

bool PermissionBinding::allowed() const
{
    return !active() || state() == PermissionEvaluator::State::Allow;
}

bool PermissionBinding::visible() const
{
    return !active() || state() != PermissionEvaluator::State::Hidden;
}

bool PermissionBinding::enabled() const
{
    if (!active())
        return true;

    const auto value = state();
    return value == PermissionEvaluator::State::Allow
        || value == PermissionEvaluator::State::ReadOnly;
}

bool PermissionBinding::editable() const
{
    return !active() || state() == PermissionEvaluator::State::Allow;
}

void PermissionBinding::reconnectEvaluator()
{
    if (!m_evaluator)
        return;

    m_providerConnection = connect(m_evaluator, &PermissionEvaluator::providerChanged,
                                   this, &PermissionBinding::stateChanged);
    m_destroyedConnection = connect(m_evaluator, &QObject::destroyed, this, [this] {
        m_evaluator = nullptr;
        m_providerConnection = {};
        m_destroyedConnection = {};
        emit evaluatorChanged();
        emit stateChanged();
    });
}

LfAttached::LfAttached(QObject *target)
    : QObject(target)
    , m_permission(new PermissionBinding(this))
{
}

LfAttached *Lf::qmlAttachedProperties(QObject *object)
{
    return new LfAttached(object);
}

} // namespace LfQuick
