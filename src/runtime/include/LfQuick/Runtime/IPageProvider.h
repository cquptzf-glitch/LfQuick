#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>
#include <LfQuick/Runtime/PageDescriptor.h>

#include <QStringView>
#include <QVariantMap>

#include <memory>

namespace LfQuick {

class IPageInstance;

struct LFQUICKRUNTIME_EXPORT PageProviderError
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

class LFQUICKRUNTIME_EXPORT IPageProvider
{
public:
    virtual ~IPageProvider() = default;
    virtual QString id() const = 0;
    virtual bool canCreate(const PageDescriptor &descriptor) const = 0;
    virtual std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                                   const QVariantMap &parameters) = 0;

    // Optional background preparation. Providers may compile/incubate/cache a
    // candidate without changing the router's synchronous navigation contract.
    // create() should consume a compatible prewarmed candidate when possible.
    virtual bool prewarm(const PageDescriptor &descriptor,
                         const QVariantMap &parameters);
    virtual void cancelPrewarm(QStringView route);

    // Legacy string error access remains for source compatibility. New code
    // should prefer lastErrorInfo() so diagnostics keep stable machine-readable
    // codes and provider-specific details.
    virtual QString lastError() const { return {}; }
    virtual PageProviderError lastErrorInfo() const;

    // Optional provider-native final fallback. QuickPageProvider uses this for
    // PageLoadError.qml; other providers can leave the default implementation.
    virtual std::unique_ptr<IPageInstance> createErrorFallback(
        const PageDescriptor &descriptor,
        const QVariantMap &parameters,
        const QString &primaryError);
};

} // namespace LfQuick
