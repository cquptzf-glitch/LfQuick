#include <LfQuick/Ui/QmlErrorCollector.h>

#include <QDateTime>

namespace LfQuick {

QmlErrorCollector::QmlErrorCollector(QObject *parent)
    : QObject(parent)
{
}

int QmlErrorCollector::count() const noexcept
{
    return m_entries.size();
}

QVariantList QmlErrorCollector::entries() const
{
    return m_entries;
}

QVariantMap QmlErrorCollector::lastError() const
{
    return m_entries.isEmpty() ? QVariantMap{} : m_entries.constLast().toMap();
}

void QmlErrorCollector::record(const QString &route,
                               const QUrl &source,
                               const QString &message,
                               const QString &stage)
{
    QVariantMap entry;
    entry.insert(QStringLiteral("route"), route);
    entry.insert(QStringLiteral("source"), source);
    entry.insert(QStringLiteral("message"), message);
    entry.insert(QStringLiteral("stage"), stage);
    entry.insert(QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc());

    m_entries.push_back(entry);
    emit errorRecorded(entry);
    emit changed();
}

void QmlErrorCollector::clear()
{
    if (m_entries.isEmpty())
        return;

    m_entries.clear();
    emit changed();
}

} // namespace LfQuick
