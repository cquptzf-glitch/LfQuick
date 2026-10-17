#pragma once

#include <LfQuick/Core/Telemetry.h>
#include <LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h>

#include <QObject>
#include <QPointer>

namespace LfQuick {

class DiagnosticsModel;

class LFQUICKDIAGNOSTICS_EXPORT DiagnosticsLogSink final : public QObject, public ILogSink
{
    Q_OBJECT
    Q_PROPERTY(DiagnosticsModel* model READ model WRITE setModel NOTIFY modelChanged FINAL)

public:
    explicit DiagnosticsLogSink(QObject *parent = nullptr);
    explicit DiagnosticsLogSink(DiagnosticsModel *model, QObject *parent = nullptr);

    DiagnosticsModel *model() const noexcept { return m_model; }
    void setModel(DiagnosticsModel *model);

    void write(const TelemetryEvent &event) override;

signals:
    void modelChanged();

private:
    QPointer<DiagnosticsModel> m_model;
};

} // namespace LfQuick
