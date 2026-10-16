#include <LfQuick/Core/Validation.h>

#include <QRegularExpression>

namespace LfQuick {

namespace {

ValidationResult invalid(const QString &message)
{
    return {false, message};
}

bool isEmptyValue(const QVariant &value)
{
    if (!value.isValid() || value.isNull())
        return true;
    if (value.metaType().id() == QMetaType::QString)
        return value.toString().trimmed().isEmpty();
    return false;
}

} // namespace

ValidationResult validateValue(const QVariant &value, const ValidationRule &rule)
{
    const bool empty = isEmptyValue(value);
    if (empty) {
        if (rule.required)
            return invalid(QStringLiteral("Value is required"));
        return {};
    }

    const QString text = value.toString();

    if (rule.minLength >= 0 && text.size() < rule.minLength)
        return invalid(QStringLiteral("Minimum length is %1").arg(rule.minLength));

    if (rule.maxLength >= 0 && text.size() > rule.maxLength)
        return invalid(QStringLiteral("Maximum length is %1").arg(rule.maxLength));

    if (!rule.regularExpression.isEmpty()) {
        const QRegularExpression expression(
            QRegularExpression::anchoredPattern(rule.regularExpression));
        if (!expression.isValid())
            return invalid(QStringLiteral("Validation rule regular expression is invalid"));
        if (!expression.match(text).hasMatch())
            return invalid(QStringLiteral("Value does not match the required format"));
    }

    if (rule.minimum.isValid() || rule.maximum.isValid() || rule.precision >= 0) {
        bool valueOk = false;
        const double numericValue = text.toDouble(&valueOk);
        if (!valueOk)
            return invalid(QStringLiteral("Value must be numeric"));

        if (rule.minimum.isValid()) {
            bool minimumOk = false;
            const double minimum = rule.minimum.toString().toDouble(&minimumOk);
            if (!minimumOk)
                return invalid(QStringLiteral("Validation rule minimum is not numeric"));
            if (numericValue < minimum)
                return invalid(QStringLiteral("Value must be at least %1").arg(rule.minimum.toString()));
        }

        if (rule.maximum.isValid()) {
            bool maximumOk = false;
            const double maximum = rule.maximum.toString().toDouble(&maximumOk);
            if (!maximumOk)
                return invalid(QStringLiteral("Validation rule maximum is not numeric"));
            if (numericValue > maximum)
                return invalid(QStringLiteral("Value must be at most %1").arg(rule.maximum.toString()));
        }

        if (rule.precision >= 0) {
            const qsizetype decimalPoint = text.indexOf(QLatin1Char('.'));
            const int actualPrecision = decimalPoint < 0
                ? 0
                : static_cast<int>(text.size() - decimalPoint - 1);
            if (actualPrecision > rule.precision)
                return invalid(QStringLiteral("Maximum precision is %1").arg(rule.precision));
        }
    }

    return {};
}

} // namespace LfQuick
