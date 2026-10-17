#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>
#include <LfQuick/Runtime/WindowDescriptor.h>

#include <QString>
#include <QVariantMap>

#include <memory>

namespace LfQuick {

class IWindowBacking;
class IWindowInstance;

struct LFQUICKRUNTIME_EXPORT WindowProviderError
{
    QString code;
    QString message;
    QVariantMap details;

    bool isEmpty() const noexcept { return code.isEmpty() && message.isEmpty() && details.isEmpty(); }
    QVariantMap toVariantMap() const
    {
        return {
            {QStringLiteral("code"), code},
            {QStringLiteral("message"), message},
            {QStringLiteral("details"), details}
        };
    }
};

class LFQUICKRUNTIME_EXPORT IWindowProvider
{
public:
    virtual ~IWindowProvider() = default;

    virtual QString id() const = 0;
    virtual bool canCreate(const WindowDescriptor &descriptor) const = 0;
    virtual std::unique_ptr<IWindowInstance> create(const WindowDescriptor &descriptor,
                                                     const WindowOpenRequest &request) = 0;

    // Optional backing-adoption contract used by WindowRouter::reloadInstance.
    // A provider must only report true for opaque backings it can safely own.
    virtual bool canAdoptBacking(const WindowDescriptor &descriptor,
                                 const IWindowBacking &backing) const;

    // Failure must leave `backing` untouched so Runtime can roll back to the
    // old logical instance. Success must consume/reset `backing` and return an
    // instance whose instanceId() is identical to backing.instanceId().
    virtual std::unique_ptr<IWindowInstance> createWithBacking(
        const WindowDescriptor &descriptor,
        const WindowOpenRequest &request,
        std::unique_ptr<IWindowBacking> &backing);

    virtual QString lastError() const { return {}; }
    virtual WindowProviderError lastErrorInfo() const;
};

} // namespace LfQuick
