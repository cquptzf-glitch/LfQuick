#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QString>
#include <QStringView>
#include <QVariant>

namespace LfQuick {

struct LFQUICKCORE_EXPORT ValidationRule
{
    QString id;
    bool required = false;
    int minLength = -1;
    int maxLength = -1;
    QString regularExpression;
    QVariant minimum;
    QVariant maximum;
    int precision = -1;
};

struct LFQUICKCORE_EXPORT ValidationResult
{
    bool valid = true;
    QString message;
};

LFQUICKCORE_EXPORT ValidationResult validateValue(const QVariant &value,
                                                   const ValidationRule &rule);

class LFQUICKCORE_EXPORT IValidationRuleProvider
{
public:
    virtual ~IValidationRuleProvider() = default;
    virtual ValidationRule rule(QStringView id) const = 0;
};

} // namespace LfQuick
