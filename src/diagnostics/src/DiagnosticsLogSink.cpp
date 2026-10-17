#include <LfQuick/Diagnostics/DiagnosticsLogSink.h>

#include <QMetaObject>
#include <QThread>

namespace LfQuick {

DiagnosticsLogSink::DiagnosticsLogSink(QObject *parent)
    : QObject(parent)
{
}

DiagnosticsLogSink::DiagnosticsLogSink(DiagnosticsModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
}

void DiagnosticsLogSink::setModel(DiagnosticsModel *model)
{
    if (m_model == model)
        return;
    m_model = model;
    emit modelChanged();
}

void DiagnosticsLogSink::write(const TelemetryEvent &event)
{
    if (!event.isValid())
        return;

    // ILogSink may be called by worker threads. Serialize access to the sink's
    // QObject/QPointer state on the sink thread before resolving the model.
    if (QThread::currentThread() != thread()) {
        const TelemetryEvent copy = event;
        QMetaObject::invokeMethod(this, [this, copy] {
            write(copy);
        }, Qt::QueuedConnection);
        return;
    }

    const QPointer<DiagnosticsModel> model = m_model;
    if (!model)
        return;

    QVariantMap details = event.attributes;
    details.insert(QStringLiteral("telemetryTimestampUtc"), event.timestamp);
    if (event.durationMs >= 0)
        details.insert(QStringLiteral("durationMs"), event.durationMs);
    if (!event.correlationId.isEmpty())
        details.insert(QStringLiteral("correlationId"), event.correlationId);

    const QString category = event.category.trimmed().isEmpty()
        ? QStringLiteral("telemetry")
        : event.category;
    const QString name = event.name;
    const QString severity = telemetryLevelName(event.level);
    const QString message = event.durationMs >= 0
        ? QStringLiteral("%1 ms").arg(event.durationMs)
        : QString{};

    const auto record = [model, category, name, details, severity, message] {
        if (model)
            model->recordEvent(category, name, details, severity, message);
    };

    if (QThread::currentThread() == model->thread())
        record();
    else
        QMetaObject::invokeMethod(model, record, Qt::QueuedConnection);
}

} // namespace LfQuick
