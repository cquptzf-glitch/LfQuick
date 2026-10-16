#pragma once

#include <LfQuick/Runtime/PageDescriptor.h>

#include <memory>

namespace LfQuick {

class IPageInstance;

class IPageProvider
{
public:
    virtual ~IPageProvider() = default;
    virtual QString id() const = 0;
    virtual bool canCreate(const PageDescriptor &descriptor) const = 0;
    virtual std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                                   const QVariantMap &parameters) = 0;

    // Providers may expose a technology-specific creation error to the generic
    // router without leaking implementation types into Runtime.
    virtual QString lastError() const { return {}; }

    // Optional provider-native final fallback. QuickPageProvider uses this for
    // PageLoadError.qml; other providers can leave the default implementation.
    virtual std::unique_ptr<IPageInstance> createErrorFallback(
        const PageDescriptor &descriptor,
        const QVariantMap &parameters,
        const QString &primaryError);
};

} // namespace LfQuick
