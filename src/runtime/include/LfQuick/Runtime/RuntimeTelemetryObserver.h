#pragma once

#include <LfQuick/Core/Telemetry.h>
#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QList>
#include <QObject>
#include <QPointer>

#include <memory>

namespace LfQuick {

class PageRouter;

class LFQUICKRUNTIME_EXPORT RuntimeTelemetryObserver : public QObject
{
    Q_OBJECT

public:
    explicit RuntimeTelemetryObserver(QObject *parent = nullptr);

    PageRouter *router() const noexcept { return m_router; }
    void setRouter(PageRouter *router);

    std::shared_ptr<ILogSink> sink() const { return m_sink; }
    void setSink(std::shared_ptr<ILogSink> sink);

private:
    void reconnect();
    void write(QString name,
               TelemetryLevel level,
               QVariantMap attributes = {},
               qint64 durationMs = -1) const;

    QPointer<PageRouter> m_router;
    std::shared_ptr<ILogSink> m_sink;
    QList<QMetaObject::Connection> m_connections;
};

} // namespace LfQuick
