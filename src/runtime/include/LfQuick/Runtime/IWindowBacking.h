#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>
#include <LfQuick/Runtime/WindowDescriptor.h>

#include <QString>

namespace LfQuick {

// Opaque ownership token for a provider's physical/native window. Runtime does
// not know whether the backing is a QWidget, QWindow, QQuickWindow or another
// technology. A backing exists so a logical IWindowInstance can be recreated
// without tearing down the expensive/native window underneath it.
class LFQUICKRUNTIME_EXPORT IWindowBacking
{
public:
    virtual ~IWindowBacking() = default;

    // Provider id prevents accidental cross-provider adoption.
    virtual QString providerId() const = 0;

    // Backing transfer is identity-preserving. Recreated logical instances
    // must expose this same instance id so owner relations and external handles
    // remain valid throughout a reload/rebind.
    virtual WindowInstanceId instanceId() const = 0;
};

} // namespace LfQuick
