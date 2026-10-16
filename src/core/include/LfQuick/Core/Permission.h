#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QStringView>
#include <QVariantMap>

namespace LfQuick {

enum class PermissionState
{
    Allow,
    ReadOnly,
    Disabled,
    Hidden,
    Denied
};

class LFQUICKCORE_EXPORT IPermissionProvider
{
public:
    IPermissionProvider();
    virtual ~IPermissionProvider();

    virtual PermissionState evaluate(QStringView permissionId,
                                     const QVariantMap &context) const = 0;
};

} // namespace LfQuick
