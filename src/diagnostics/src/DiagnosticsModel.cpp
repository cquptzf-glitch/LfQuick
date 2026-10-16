#include <LfQuick/Diagnostics/DiagnosticsModel.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <algorithm>

namespace LfQuick {

DiagnosticsModel::DiagnosticsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int DiagnosticsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_events.size();
}

QVariant DiagnosticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_events.size())
        return {};

    const auto &event = m_events.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return event.message.isEmpty() ? event.name : event.message;
    case SequenceRole:
        return event.sequence;
    case TimestampRole:
        return event.timestampUtc;
    case TimeTextRole:
        return event.timestampUtc.toLocalTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    case CategoryRole:
        return event.category;
    case NameRole:
        return event.name;
    case SeverityRole:
        return event.severity;
    case MessageRole:
        return event.message;
    case DetailsRole:
        return event.details;
    default:
        return {};
    }
}

QHash<int, QByteArray> DiagnosticsModel::roleNames() const
{
    return {
        {SequenceRole, "sequence"},
        {TimestampRole, "timestamp"},
        {TimeTextRole, "timeText"},
        {CategoryRole, "category"},
        {NameRole, "name"},
        {SeverityRole, "severity"},
        {MessageRole, "message"},
        {DetailsRole, "details"}
    };
}

void DiagnosticsModel::setMaxEvents(int value)
{
    value = std::clamp(value, 1, 100000);
    if (m_maxEvents == value)
        return;

    m_maxEvents = value;
    emit maxEventsChanged();
    trimToCapacity();
}

void DiagnosticsModel::recordEvent(const QString &category,
                                   const QString &name,
                                   const QVariantMap &details,
                                   const QString &severity,
                                   const QString &message)
{
    const QString normalizedCategory = category.trimmed().isEmpty()
        ? QStringLiteral("general")
        : category.trimmed();
    const QString normalizedName = name.trimmed().isEmpty()
        ? QStringLiteral("event")
        : name.trimmed();
    const QString normalizedSeverity = severity.trimmed().isEmpty()
        ? QStringLiteral("info")
        : severity.trimmed().toLower();

    trimToCapacity(1);

    Event event;
    event.sequence = m_nextSequence++;
    event.timestampUtc = QDateTime::currentDateTimeUtc();
    event.category = normalizedCategory;
    event.name = normalizedName;
    event.severity = normalizedSeverity;
    event.message = message;
    event.details = details;

    const int row = m_events.size();
    beginInsertRows({}, row, row);
    m_events.push_back(event);
    endInsertRows();
    emit countChanged();
    emit eventRecorded(eventToMap(event));
}

void DiagnosticsModel::recordTiming(const QString &key,
                                    double milliseconds,
                                    const QVariantMap &details)
{
    if (key.trimmed().isEmpty())
        return;

    setMetric(key, milliseconds);
    QVariantMap timingDetails = details;
    timingDetails.insert(QStringLiteral("key"), key);
    timingDetails.insert(QStringLiteral("milliseconds"), milliseconds);
    recordEvent(QStringLiteral("timing"), key, timingDetails,
                QStringLiteral("info"),
                QStringLiteral("%1 ms").arg(milliseconds, 0, 'f', 2));
}

void DiagnosticsModel::setMetric(const QString &key, const QVariant &value)
{
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty())
        return;

    const auto it = m_metrics.constFind(normalizedKey);
    if (it != m_metrics.cend() && it.value() == value)
        return;

    m_metrics.insert(normalizedKey, value);
    emit metricsChanged();
}

QVariant DiagnosticsModel::metric(const QString &key) const
{
    return m_metrics.value(key);
}

qint64 DiagnosticsModel::incrementMetric(const QString &key, qint64 delta)
{
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty())
        return 0;

    bool ok = false;
    const qint64 current = m_metrics.value(normalizedKey).toLongLong(&ok);
    const qint64 next = (ok ? current : 0) + delta;
    setMetric(normalizedKey, next);
    return next;
}

bool DiagnosticsModel::removeMetric(const QString &key)
{
    if (m_metrics.remove(key.trimmed()) <= 0)
        return false;
    emit metricsChanged();
    return true;
}

QVariantMap DiagnosticsModel::eventAt(int row) const
{
    if (row < 0 || row >= m_events.size())
        return {};
    return eventToMap(m_events.at(row));
}

QVariantMap DiagnosticsModel::snapshot() const
{
    QVariantList events;
    events.reserve(m_events.size());
    for (const auto &event : m_events)
        events.push_back(eventToMap(event));

    return {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("generatedAtUtc"),
         QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("maxEvents"), m_maxEvents},
        {QStringLiteral("droppedCount"), m_droppedCount},
        {QStringLiteral("metrics"), m_metrics},
        {QStringLiteral("events"), events}
    };
}

QString DiagnosticsModel::toJson(bool indented) const
{
    const QJsonDocument document(QJsonObject::fromVariantMap(snapshot()));
    return QString::fromUtf8(document.toJson(indented
                                                 ? QJsonDocument::Indented
                                                 : QJsonDocument::Compact));
}

bool DiagnosticsModel::exportJson(const QString &filePath)
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        setLastExportError(QStringLiteral("Export file path is empty"));
        return false;
    }

    QSaveFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setLastExportError(file.errorString());
        return false;
    }

    const QByteArray payload = toJson(true).toUtf8();
    if (file.write(payload) != payload.size()) {
        setLastExportError(file.errorString());
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        setLastExportError(file.errorString());
        return false;
    }

    setLastExportError({});
    emit exported(normalizedPath);
    return true;
}

void DiagnosticsModel::clearEvents()
{
    if (m_events.isEmpty())
        return;

    beginResetModel();
    m_events.clear();
    endResetModel();
    emit countChanged();
}

void DiagnosticsModel::clearMetrics()
{
    if (m_metrics.isEmpty())
        return;
    m_metrics.clear();
    emit metricsChanged();
}

void DiagnosticsModel::reset()
{
    const bool hadEvents = !m_events.isEmpty();
    const bool hadMetrics = !m_metrics.isEmpty();
    const bool hadDropped = m_droppedCount != 0;

    if (hadEvents) {
        beginResetModel();
        m_events.clear();
        endResetModel();
        emit countChanged();
    }
    if (hadMetrics) {
        m_metrics.clear();
        emit metricsChanged();
    }
    if (hadDropped) {
        m_droppedCount = 0;
        emit droppedCountChanged();
    }
    m_nextSequence = 1;
    setLastExportError({});
}

QVariantMap DiagnosticsModel::eventToMap(const Event &event) const
{
    return {
        {QStringLiteral("sequence"), event.sequence},
        {QStringLiteral("timestampUtc"), event.timestampUtc.toString(Qt::ISODateWithMs)},
        {QStringLiteral("category"), event.category},
        {QStringLiteral("name"), event.name},
        {QStringLiteral("severity"), event.severity},
        {QStringLiteral("message"), event.message},
        {QStringLiteral("details"), event.details}
    };
}

void DiagnosticsModel::trimToCapacity(int incomingCount)
{
    const int overflow = m_events.size() + incomingCount - m_maxEvents;
    if (overflow <= 0)
        return;

    const int removeCount = std::min(overflow, m_events.size());
    if (removeCount <= 0)
        return;

    beginRemoveRows({}, 0, removeCount - 1);
    m_events.erase(m_events.begin(), m_events.begin() + removeCount);
    endRemoveRows();
    m_droppedCount += removeCount;
    emit countChanged();
    emit droppedCountChanged();
}

void DiagnosticsModel::setLastExportError(const QString &error)
{
    if (m_lastExportError == error)
        return;
    m_lastExportError = error;
    emit lastExportErrorChanged();
}

} // namespace LfQuick
