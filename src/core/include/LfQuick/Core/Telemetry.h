#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QDateTime>
#include <QString>
#include <QVariantMap>

#include <memory>

namespace LfQuick {

enum class TelemetryLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

struct LFQUICKCORE_EXPORT TelemetryEvent
{
    QString name;
    QString category;
    TelemetryLevel level = TelemetryLevel::Info;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
    QVariantMap attributes;
    qint64 durationMs = -1;
    QString correlationId;

    bool isValid() const noexcept { return !name.trimmed().isEmpty(); }
    QVariantMap toVariantMap() const;
};

class LFQUICKCORE_EXPORT ILogSink
{
public:
    virtual ~ILogSink() = default;
    virtual void write(const TelemetryEvent &event) = 0;
};

LFQUICKCORE_EXPORT QString telemetryLevelName(TelemetryLevel level);

} // namespace LfQuick
