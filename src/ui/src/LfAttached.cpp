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

void LfAttached::setValidationRule(const QVariantMap &rule)
{
    if (m_validationRule == rule)
        return;
    m_validationRule = rule;
    emit validationRuleChanged();
}

void LfAttached::setFocusPolicy(int policy)
{
    const int normalized = policy < Lf::Auto || policy > Lf::Grid ? Lf::Auto : policy;
    if (m_focusPolicy == normalized)
        return;
    m_focusPolicy = normalized;
    emit focusPolicyChanged();
}

void LfAttached::setFocusOrder(int order)
{
    if (m_focusOrder == order)
        return;
    m_focusOrder = order;
    emit focusOrderChanged();
}

void LfAttached::setAcceptScanner(bool value)
{
    if (m_acceptScanner == value)
        return;
    m_acceptScanner = value;
    emit acceptScannerChanged();
}

void LfAttached::setHelpContext(const QString &context)
{
    const QString normalized = context.trimmed();
    if (m_helpContext == normalized)
        return;
    m_helpContext = normalized;
    emit helpContextChanged();
}

void LfAttached::setReloadKey(const QString &key)
{
    const QString normalized = key.trimmed();
    if (m_reloadKey == normalized)
        return;
    m_reloadKey = normalized;
    emit reloadKeyChanged();
}

void LfAttached::setStateProperties(const QStringList &properties)
{
    QStringList normalized;
    normalized.reserve(properties.size());
    for (const QString &property : properties) {
        const QString name = property.trimmed();
        if (!name.isEmpty() && !normalized.contains(name))
            normalized.push_back(name);
    }

    if (m_stateProperties == normalized)
        return;
    m_stateProperties = normalized;
    emit statePropertiesChanged();
}

LfAttached *Lf::qmlAttachedProperties(QObject *object)
{
    return new LfAttached(object);
}

} // namespace LfQuick
