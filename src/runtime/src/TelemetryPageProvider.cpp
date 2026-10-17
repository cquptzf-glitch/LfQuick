#include <LfQuick/Runtime/TelemetryPageProvider.h>

#include <QElapsedTimer>

namespace LfQuick {

TelemetryPageProvider::TelemetryPageProvider(std::shared_ptr<IPageProvider> provider,
                                             std::shared_ptr<ILogSink> sink)
    : m_provider(std::move(provider))
    , m_sink(std::move(sink))
{
}

QString TelemetryPageProvider::id() const
{
    return m_provider ? m_provider->id() : QString{};
}

bool TelemetryPageProvider::canCreate(const PageDescriptor &descriptor) const
{
    return m_provider && m_provider->canCreate(descriptor);
}

std::unique_ptr<IPageInstance> TelemetryPageProvider::create(
    const PageDescriptor &descriptor,
    const QVariantMap &parameters)
{
    if (!m_provider)
        return {};

    QElapsedTimer timer;
    timer.start();
    auto instance = m_provider->create(descriptor, parameters);
    const QString error = instance ? QString{} : m_provider->lastErrorInfo().message;
    writeTiming(QStringLiteral("create"), descriptor, timer.elapsed(), bool(instance), error);
    return instance;
}

QString TelemetryPageProvider::lastError() const
{
    return m_provider ? m_provider->lastError() : QString{};
}

PageProviderError TelemetryPageProvider::lastErrorInfo() const
{
    return m_provider ? m_provider->lastErrorInfo() : PageProviderError{};
}

std::unique_ptr<IPageInstance> TelemetryPageProvider::createErrorFallback(
    const PageDescriptor &descriptor,
    const QVariantMap &parameters,
    const QString &primaryError)
{
    if (!m_provider)
        return {};

    QElapsedTimer timer;
    timer.start();
    auto instance = m_provider->createErrorFallback(descriptor, parameters, primaryError);
    const QString error = instance ? QString{} : m_provider->lastErrorInfo().message;
    writeTiming(QStringLiteral("error-fallback"), descriptor,
                timer.elapsed(), bool(instance), error);
    return instance;
}

void TelemetryPageProvider::writeTiming(const QString &operation,
                                        const PageDescriptor &descriptor,
                                        qint64 durationMs,
                                        bool success,
                                        const QString &error) const
{
    if (!m_sink)
        return;

    TelemetryEvent event;
    event.name = QStringLiteral("runtime.page.%1").arg(operation);
    event.category = QStringLiteral("runtime.page-provider");
    event.level = success ? TelemetryLevel::Info : TelemetryLevel::Error;
    event.durationMs = durationMs;
    event.attributes = {
        {QStringLiteral("route"), descriptor.route},
        {QStringLiteral("provider"), id()},
        {QStringLiteral("success"), success},
        {QStringLiteral("cachePolicy"), static_cast<int>(descriptor.cachePolicy)},
        {QStringLiteral("featureSwitch"), static_cast<int>(descriptor.featureSwitch)}
    };
    if (!error.trimmed().isEmpty())
        event.attributes.insert(QStringLiteral("error"), error.trimmed());
    m_sink->write(event);
}

} // namespace LfQuick
