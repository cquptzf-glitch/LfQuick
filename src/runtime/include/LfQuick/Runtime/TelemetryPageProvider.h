#pragma once

#include <LfQuick/Core/Telemetry.h>
#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <memory>

namespace LfQuick {

class LFQUICKRUNTIME_EXPORT TelemetryPageProvider final : public IPageProvider
{
public:
    TelemetryPageProvider(std::shared_ptr<IPageProvider> provider,
                          std::shared_ptr<ILogSink> sink);

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;
    QString lastError() const override;
    PageProviderError lastErrorInfo() const override;
    std::unique_ptr<IPageInstance> createErrorFallback(
        const PageDescriptor &descriptor,
        const QVariantMap &parameters,
        const QString &primaryError) override;

    std::shared_ptr<IPageProvider> provider() const { return m_provider; }
    std::shared_ptr<ILogSink> sink() const { return m_sink; }

private:
    void writeTiming(const QString &operation,
                     const PageDescriptor &descriptor,
                     qint64 durationMs,
                     bool success,
                     const QString &error = {}) const;

    std::shared_ptr<IPageProvider> m_provider;
    std::shared_ptr<ILogSink> m_sink;
};

} // namespace LfQuick
