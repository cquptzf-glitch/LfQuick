#pragma once

#include <LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h>

#include <QAbstractListModel>
#include <QDateTime>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDIAGNOSTICS_EXPORT DiagnosticsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int maxEvents READ maxEvents WRITE setMaxEvents NOTIFY maxEventsChanged FINAL)
    Q_PROPERTY(qint64 droppedCount READ droppedCount NOTIFY droppedCountChanged FINAL)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY metricsChanged FINAL)
    Q_PROPERTY(QString lastExportError READ lastExportError NOTIFY lastExportErrorChanged FINAL)

public:
    enum Role {
        SequenceRole = Qt::UserRole + 1,
        TimestampRole,
        TimeTextRole,
        CategoryRole,
        NameRole,
        SeverityRole,
        MessageRole,
        DetailsRole
    };
    Q_ENUM(Role)

    explicit DiagnosticsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const noexcept { return m_events.size(); }
    int maxEvents() const noexcept { return m_maxEvents; }
    void setMaxEvents(int value);
    qint64 droppedCount() const noexcept { return m_droppedCount; }
    QVariantMap metrics() const { return m_metrics; }
    QString lastExportError() const { return m_lastExportError; }

    Q_INVOKABLE void recordEvent(const QString &category,
                                 const QString &name,
                                 const QVariantMap &details = {},
                                 const QString &severity = QStringLiteral("info"),
                                 const QString &message = {});
    Q_INVOKABLE void recordTiming(const QString &key,
                                  double milliseconds,
                                  const QVariantMap &details = {});

    Q_INVOKABLE void setMetric(const QString &key, const QVariant &value);
    Q_INVOKABLE QVariant metric(const QString &key) const;
    Q_INVOKABLE qint64 incrementMetric(const QString &key, qint64 delta = 1);
    Q_INVOKABLE bool removeMetric(const QString &key);

    Q_INVOKABLE QVariantMap eventAt(int row) const;
    Q_INVOKABLE QVariantMap snapshot() const;
    Q_INVOKABLE QString toJson(bool indented = true) const;
    Q_INVOKABLE bool exportJson(const QString &filePath);

    Q_INVOKABLE void clearEvents();
    Q_INVOKABLE void clearMetrics();
    Q_INVOKABLE void reset();

signals:
    void countChanged();
    void maxEventsChanged();
    void droppedCountChanged();
    void metricsChanged();
    void lastExportErrorChanged();
    void eventRecorded(const QVariantMap &event);
    void exported(const QString &filePath);

private:
    struct Event {
        qint64 sequence = 0;
        QDateTime timestampUtc;
        QString category;
        QString name;
        QString severity;
        QString message;
        QVariantMap details;
    };

    QVariantMap eventToMap(const Event &event) const;
    void trimToCapacity(int incomingCount = 0);
    void setLastExportError(const QString &error);

    QList<Event> m_events;
    QVariantMap m_metrics;
    int m_maxEvents = 250;
    qint64 m_droppedCount = 0;
    qint64 m_nextSequence = 1;
    QString m_lastExportError;
};

} // namespace LfQuick
