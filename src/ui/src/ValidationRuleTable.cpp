#include <LfQuick/Ui/ValidationRuleTable.h>

#include <QStringView>

#include <utility>

namespace LfQuick {

ValidationRuleTable::ValidationRuleTable(QObject *parent)
    : QObject(parent)
{
}

void ValidationRuleTable::setRules(const QVariantMap &rules)
{
    QVariantMap normalized;
    for (auto it = rules.cbegin(); it != rules.cend(); ++it) {
        const QString id = it.key().trimmed();
        if (id.isEmpty() || !it.value().canConvert<QVariantMap>())
            continue;
        QVariantMap ruleMap = it.value().toMap();
        ruleMap.insert(QStringLiteral("id"), id);
        normalized.insert(id, ruleMap);
    }

    if (m_rules == normalized)
        return;
    m_rules = std::move(normalized);
    emit rulesChanged();
}

void ValidationRuleTable::setProvider(std::shared_ptr<IValidationRuleProvider> provider)
{
    if (m_provider == provider)
        return;
    m_provider = std::move(provider);
    emit providerChanged();
    emit rulesChanged();
}

bool ValidationRuleTable::setRule(const QString &ruleId, const QVariantMap &rule)
{
    const QString id = ruleId.trimmed();
    if (id.isEmpty())
        return false;

    QVariantMap normalized = rule;
    normalized.insert(QStringLiteral("id"), id);
    if (m_rules.value(id).toMap() == normalized)
        return true;

    m_rules.insert(id, normalized);
    emit rulesChanged();
    return true;
}

bool ValidationRuleTable::removeRule(const QString &ruleId)
{
    const QString id = ruleId.trimmed();
    if (id.isEmpty() || !m_rules.contains(id))
        return false;
    m_rules.remove(id);
    emit rulesChanged();
    return true;
}

void ValidationRuleTable::clear()
{
    if (m_rules.isEmpty())
        return;
    m_rules.clear();
    emit rulesChanged();
}

bool ValidationRuleTable::contains(const QString &ruleId) const
{
    return !resolvedRule(ruleId).isEmpty();
}

QVariantMap ValidationRuleTable::rule(const QString &ruleId) const
{
    return resolvedRule(ruleId);
}

QVariantMap ValidationRuleTable::validate(const QString &ruleId, const QVariant &value) const
{
    const QString id = ruleId.trimmed();
    const QVariantMap ruleMap = resolvedRule(id);
    if (id.isEmpty() || ruleMap.isEmpty()) {
        return {
            {QStringLiteral("valid"), false},
            {QStringLiteral("message"), QStringLiteral("Validation rule not found: %1").arg(id)},
            {QStringLiteral("ruleId"), id}
        };
    }

    const ValidationRule parsed = ruleFromMap(ruleMap);
    const ValidationResult result = validateValue(value, parsed);
    return {
        {QStringLiteral("valid"), result.valid},
        {QStringLiteral("message"), result.message},
        {QStringLiteral("ruleId"), parsed.id}
    };
}

bool ValidationRuleTable::isValid(const QString &ruleId, const QVariant &value) const
{
    return validate(ruleId, value).value(QStringLiteral("valid")).toBool();
}

ValidationRule ValidationRuleTable::ruleFromMap(const QVariantMap &map)
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

QVariantMap ValidationRuleTable::ruleToMap(const ValidationRule &rule)
{
    return {
        {QStringLiteral("id"), rule.id},
        {QStringLiteral("required"), rule.required},
        {QStringLiteral("minLength"), rule.minLength},
        {QStringLiteral("maxLength"), rule.maxLength},
        {QStringLiteral("regularExpression"), rule.regularExpression},
        {QStringLiteral("minimum"), rule.minimum},
        {QStringLiteral("maximum"), rule.maximum},
        {QStringLiteral("precision"), rule.precision}
    };
}

QVariantMap ValidationRuleTable::resolvedRule(const QString &ruleId) const
{
    const QString id = ruleId.trimmed();
    if (id.isEmpty())
        return {};

    const auto local = m_rules.constFind(id);
    if (local != m_rules.cend())
        return local.value().toMap();

    if (!m_provider)
        return {};

    ValidationRule providerRule = m_provider->rule(QStringView{id});
    if (providerRule.id.trimmed().isEmpty())
        return {};
    return ruleToMap(providerRule);
}

} // namespace LfQuick
