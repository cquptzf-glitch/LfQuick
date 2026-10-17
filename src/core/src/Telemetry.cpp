#include <LfQuick/Core/Telemetry.h>

namespace LfQuick {

QString telemetryLevelName(TelemetryLevel level)
{
    switch (level) {
    case TelemetryLevel::Trace:
        return QStringLiteral("trace");
    case TelemetryLevel::Debug:
        return QStringLiteral("debug");
    case TelemetryLevel::Warning:
        return QStringLiteral("warning");
    case TelemetryLevel::Error:
        return QStringLiteral("error");
    case TelemetryLevel::Critical:
        return QStringLiteral("critical");
    case TelemetryLevel::Info:
    default:
        return QStringLiteral("info");
    }
}

QVariantMap TelemetryEvent::toVariantMap() const
{
    QVariantMap result{
        {QStringLiteral("name"), name},
        {QStringLiteral("category"), category},
        {QStringLiteral("level"), telemetryLevelName(level)},
        {QStringLiteral("timestamp"), timestamp.toUTC().toString(Qt::ISODateWithMs)},
        {QStringLiteral("attributes"), attributes}
    };

    if (durationMs >= 0)
        result.insert(QStringLiteral("durationMs"), durationMs);
    if (!correlationId.isEmpty())
        result.insert(QStringLiteral("correlationId"), correlationId);
    return result;
}

} // namespace LfQuick
