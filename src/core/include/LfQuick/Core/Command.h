#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>
#include <LfQuick/Core/Result.h>

#include <QVariant>
#include <QVariantMap>

namespace LfQuick {

struct LFQUICKCORE_EXPORT CommandContext
{
    QVariantMap arguments;
    QVariantMap metadata;
};

class LFQUICKCORE_EXPORT ICommand
{
public:
    virtual ~ICommand() = default;

    virtual bool canExecute(const CommandContext &context = {}) const = 0;
    virtual Result<QVariant> execute(const CommandContext &context = {}) = 0;
};

} // namespace LfQuick
