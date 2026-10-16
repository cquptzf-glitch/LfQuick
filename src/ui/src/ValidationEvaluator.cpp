#include <LfQuick/Ui/ValidationEvaluator.h>

#include <LfQuick/Core/Validation.h>

namespace LfQuick {

ValidationEvaluator::ValidationEvaluator(QObject *parent)
    : QObject(parent)
{
}

namespace {

ValidationRule ruleFromMap(const QVariantMap &map)
{
    ValidationRule rule;
    rule.id = map.value(QStringLiteral("id")).toString();
    rule.required = map.value(QStringLiteral("required"), false).toBool();
    rule.minLength = map.value(QStringLiteral("minLength"), -1).toInt();
    rule.maxLength = map.value(QStringLiteral("maxLength"), -1).toInt();
    rule.regularExpression = map.value(QStringLiteral("regularExpression")).toString();
    rule.minimum = map.value(QStringLiteral("minimum"));
    rule.maximum = map.value(QStringLiteral("maximum"));
    rule.precision = map.value(QStringLiteral("precision"), -1).toInt();
    return rule;
}

} // namespace

QVariantMap ValidationEvaluator::validate(const QVariant &value,
                                          const QVariantMap &ruleMap) const
{
    const ValidationRule rule = ruleFromMap(ruleMap);
    const ValidationResult result = validateValue(value, rule);
    return {
        {QStringLiteral("valid"), result.valid},
        {QStringLiteral("message"), result.message},
        {QStringLiteral("ruleId"), rule.id}
    };
}

bool ValidationEvaluator::isValid(const QVariant &value,
                                  const QVariantMap &ruleMap) const
{
    return validateValue(value, ruleFromMap(ruleMap)).valid;
}

} // namespace LfQuick
