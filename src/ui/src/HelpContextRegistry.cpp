#include <LfQuick/Ui/HelpContextRegistry.h>

#include <algorithm>

namespace LfQuick {

HelpContextRegistry::HelpContextRegistry(QObject *parent)
    : QObject(parent)
{
}

void HelpContextRegistry::setContexts(const QVariantMap &contexts)
{
    QVariantMap normalized;
    for (auto it = contexts.cbegin(); it != contexts.cend(); ++it) {
        const QString id = normalizedId(it.key());
        if (id.isEmpty())
            continue;
        normalized.insert(id, normalizedEntry(it.value().toMap()));
    }

    if (m_contexts == normalized)
        return;
    m_contexts = std::move(normalized);
    emit contextsChanged();
}

bool HelpContextRegistry::registerContext(const QString &idValue, const QVariantMap &entry)
{
    const QString id = normalizedId(idValue);
    if (id.isEmpty())
        return false;

    const QVariantMap value = normalizedEntry(entry);
    if (m_contexts.value(id).toMap() == value)
        return true;

    m_contexts.insert(id, value);
    emit contextsChanged();
    return true;
}

bool HelpContextRegistry::unregisterContext(const QString &idValue)
{
    const QString id = normalizedId(idValue);
    if (id.isEmpty() || !m_contexts.contains(id))
        return false;
    m_contexts.remove(id);
    emit contextsChanged();
    return true;
}

bool HelpContextRegistry::contains(const QString &id) const
{
    return m_contexts.contains(normalizedId(id));
}

QVariantMap HelpContextRegistry::resolve(const QString &id) const
{
    return m_contexts.value(normalizedId(id)).toMap();
}

QString HelpContextRegistry::title(const QString &id) const
{
    return resolve(id).value(QStringLiteral("title")).toString();
}

QString HelpContextRegistry::text(const QString &id) const
{
    return resolve(id).value(QStringLiteral("text")).toString();
}

QString HelpContextRegistry::url(const QString &id) const
{
    return resolve(id).value(QStringLiteral("url")).toString();
}

QStringList HelpContextRegistry::ids() const
{
    QStringList result = m_contexts.keys();
    std::sort(result.begin(), result.end());
    return result;
}

void HelpContextRegistry::clear()
{
    if (m_contexts.isEmpty())
        return;
    m_contexts.clear();
    emit contextsChanged();
}

bool HelpContextRegistry::requestHelp(const QString &idValue)
{
    const QString id = normalizedId(idValue);
    const QVariantMap entry = resolve(id);
    if (id.isEmpty() || entry.isEmpty())
        return false;
    emit helpRequested(id, entry);
    return true;
}

QString HelpContextRegistry::normalizedId(const QString &id)
{
    return id.trimmed();
}

QVariantMap HelpContextRegistry::normalizedEntry(const QVariantMap &entry)
{
    QVariantMap result = entry;
    result.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString().trimmed());
    result.insert(QStringLiteral("text"), entry.value(QStringLiteral("text")).toString());
    result.insert(QStringLiteral("url"), entry.value(QStringLiteral("url")).toString().trimmed());
    return result;
}

} // namespace LfQuick
