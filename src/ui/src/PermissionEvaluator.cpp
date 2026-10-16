#include <LfQuick/Ui/PermissionEvaluator.h>

namespace LfQuick {

PermissionEvaluator::PermissionEvaluator(QObject *parent)
    : QObject(parent)
{
}

void PermissionEvaluator::setProvider(std::shared_ptr<IPermissionProvider> provider)
{
    if (m_provider == provider)
        return;
    m_provider = std::move(provider);
    emit providerChanged();
}

PermissionEvaluator::State PermissionEvaluator::evaluate(
    const QString &permissionId,
    const QVariantMap &context) const
{
    if (!m_provider || permissionId.trimmed().isEmpty())
        return State::Denied;

    return static_cast<State>(m_provider->evaluate(permissionId, context));
}

bool PermissionEvaluator::allows(const QString &permissionId,
                                 const QVariantMap &context) const
{
    return evaluate(permissionId, context) == State::Allow;
}

bool PermissionEvaluator::isVisible(const QString &permissionId,
                                    const QVariantMap &context) const
{
    return evaluate(permissionId, context) != State::Hidden;
}

bool PermissionEvaluator::isEnabled(const QString &permissionId,
                                    const QVariantMap &context) const
{
    const State state = evaluate(permissionId, context);
    return state == State::Allow || state == State::ReadOnly;
}

bool PermissionEvaluator::isEditable(const QString &permissionId,
                                     const QVariantMap &context) const
{
    return evaluate(permissionId, context) == State::Allow;
}

} // namespace LfQuick
