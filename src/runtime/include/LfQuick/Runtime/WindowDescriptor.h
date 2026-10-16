#pragma once

#include <LfQuick/Core/Error.h>
#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QString>
#include <QVariantMap>

namespace LfQuick {

using WindowId = QString;
using WindowInstanceId = QString;

enum class WindowOpenPolicy {
    SingleInstance,
    MultipleInstances
};

enum class OwnerClosePolicy {
    Detach,
    CloseWithOwner
};

enum class WindowState {
    Created,
    Active,
    Inactive,
    Closing,
    Closed
};

struct LFQUICKRUNTIME_EXPORT WindowDescriptor
{
    WindowId id;
    QString provider;
    QVariantMap source;
    WindowOpenPolicy openPolicy = WindowOpenPolicy::SingleInstance;
    OwnerClosePolicy ownerClosePolicy = OwnerClosePolicy::CloseWithOwner;
    QVariantMap metadata;
};

struct LFQUICKRUNTIME_EXPORT WindowOpenRequest
{
    WindowId windowId;
    QVariantMap parameters;
    WindowInstanceId ownerInstanceId;
    QVariantMap restoreState;
    QVariantMap metadata;
};

struct LFQUICKRUNTIME_EXPORT WindowResult
{
    bool accepted = false;
    WindowId windowId;
    WindowInstanceId instanceId;
    Error error;
    QVariantMap diagnostics;

    static WindowResult success(WindowId id, WindowInstanceId instance)
    {
        WindowResult result;
        result.accepted = true;
        result.windowId = std::move(id);
        result.instanceId = std::move(instance);
        return result;
    }

    static WindowResult failure(WindowId id,
                                WindowInstanceId instance,
                                Error windowError,
                                QVariantMap windowDiagnostics = {})
    {
        WindowResult result;
        result.windowId = std::move(id);
        result.instanceId = std::move(instance);
        result.error = std::move(windowError);
        result.diagnostics = std::move(windowDiagnostics);
        return result;
    }
};

} // namespace LfQuick
