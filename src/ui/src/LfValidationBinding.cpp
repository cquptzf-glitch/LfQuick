#include <LfQuick/Ui/LfValidationBinding.h>

#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/ValidationEvaluator.h>

#include <QtQml/qqml.h>

namespace LfQuick {

LfValidationBinding::LfValidationBinding(QObject *parent)
    : QObject(parent)
    , m_evaluator(new ValidationEvaluator(this))
{
}

void LfValidationBinding::setTarget(QObject *target)
{
    if (m_target == target)
        return;

    QObject::disconnect(m_ruleConnection);
    QObject::disconnect(m_targetDestroyedConnection);
    m_ruleConnection = {};
    m_targetDestroyedConnection = {};
    m_attached = nullptr;
    m_target = target;

    reconnectTarget();
    emit targetChanged();
}

void LfValidationBinding::setValue(const QVariant &value)
{
    if (m_value == value)
        return;
    m_value = value;
    emit valueChanged();
    evaluate();
}

QVariantMap LfValidationBinding::validateNow()
{
    refreshRule();
    evaluate();
    return m_result;
}

void LfValidationBinding::reconnectTarget()
{
    if (m_target) {
        m_attached = qmlAttachedPropertiesObject<Lf>(m_target, true);
        if (m_attached) {
            m_ruleConnection = connect(m_attached, &LfAttached::validationRuleChanged,
                                       this, [this] {
                refreshRule();
                evaluate();
            });
        }
        m_targetDestroyedConnection = connect(m_target, &QObject::destroyed,
                                              this, [this] {
            m_target = nullptr;
            m_attached = nullptr;
            m_ruleConnection = {};
            m_targetDestroyedConnection = {};
            refreshRule();
            evaluate();
            emit targetChanged();
        });
    }

    refreshRule();
    evaluate();
}

void LfValidationBinding::refreshRule()
{
    const QVariantMap next = m_attached ? m_attached->validationRule() : QVariantMap{};
    if (m_rule == next)
        return;
    m_rule = next;
    emit ruleChanged();
}

void LfValidationBinding::evaluate()
{
    const QVariantMap next = m_evaluator
        ? m_evaluator->validate(m_value, m_rule)
        : QVariantMap{{QStringLiteral("valid"), true},
                      {QStringLiteral("message"), QString{}},
                      {QStringLiteral("ruleId"), QString{}}};
    if (m_result == next)
        return;
    m_result = next;
    emit resultChanged();
}

} // namespace LfQuick
