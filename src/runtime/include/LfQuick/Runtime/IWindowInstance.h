#pragma once

#include <LfQuick/Runtime/WindowDescriptor.h>

#include <QVariantMap>

#include <functional>
#include <utility>

namespace LfQuick {

class LFQUICKRUNTIME_EXPORT IWindowInstance
{
public:
    using CloseRequestHandler = std::function<void()>;

    virtual ~IWindowInstance() = default;

    virtual WindowInstanceId instanceId() const = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    virtual bool canClose(QString *reason = nullptr) const
    {
        Q_UNUSED(reason);
        return true;
    }

    // Native window systems can request a close independently from the
    // WindowRouter (for example, a title-bar close button). Providers should
    // route that request through this callback so guards, owner propagation,
    // diagnostics and router bookkeeping remain authoritative.
    virtual void setCloseRequestHandler(CloseRequestHandler handler)
    {
        Q_UNUSED(handler);
    }

    // Runtime stores ownership using technology-neutral instance ids. Native
    // adapters can override these hooks to keep QWidget/QWindow/etc ownership
    // in sync. The default accepts detaching because non-native providers may
    // have no platform ownership to update, but rejects assigning a new owner.
    virtual bool canUpdateOwner(const WindowInstanceId &ownerInstanceId) const
    {
        return ownerInstanceId.isEmpty();
    }

    virtual bool updateOwner(const WindowInstanceId &ownerInstanceId)
    {
        return ownerInstanceId.isEmpty();
    }

    virtual bool updateParameters(const QVariantMap &parameters)
    {
        return parameters.isEmpty();
    }

    virtual bool restoreState(const QVariantMap &state)
    {
        return state.isEmpty();
    }

    virtual QVariantMap saveState() const { return {}; }
    virtual void close() = 0;
};

} // namespace LfQuick
