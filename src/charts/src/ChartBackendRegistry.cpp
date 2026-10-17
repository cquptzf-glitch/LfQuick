#include <LfQuick/Charts/ChartBackendRegistry.h>

#include <algorithm>

namespace LfQuick {

namespace {

QVariantList seriesTypesToVariant(const QList<ChartSeriesType> &types)
{
    QVariantList result;
    result.reserve(types.size());
    for (const auto type : types)
        result.push_back(static_cast<int>(type));
    return result;
}

QList<ChartSeriesType> seriesTypesFromVariant(const QVariant &value)
{
    QList<ChartSeriesType> result;
    for (const QVariant &entry : value.toList()) {
        const int raw = entry.toInt();
        if (raw < static_cast<int>(ChartSeriesType::Line)
            || raw > static_cast<int>(ChartSeriesType::Area)) {
            continue;
        }
        const auto type = static_cast<ChartSeriesType>(raw);
        if (!result.contains(type))
            result.push_back(type);
    }
    return result;
}

} // namespace

bool ChartBackendDescriptor::supports(ChartSeriesType type) const
{
    return seriesTypes.isEmpty() || seriesTypes.contains(type);
}

QVariantMap ChartBackendDescriptor::toVariantMap() const
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), name},
        {QStringLiteral("technology"), technology},
        {QStringLiteral("priority"), priority},
        {QStringLiteral("available"), available},
        {QStringLiteral("seriesTypes"), seriesTypesToVariant(seriesTypes)},
        {QStringLiteral("metadata"), metadata}
    };
}

ChartBackendDescriptor ChartBackendDescriptor::fromVariantMap(const QVariantMap &map)
{
    ChartBackendDescriptor descriptor;
    descriptor.id = map.value(QStringLiteral("id")).toString().trimmed();
    descriptor.name = map.value(QStringLiteral("name"), descriptor.id).toString().trimmed();
    descriptor.technology = map.value(QStringLiteral("technology")).toString().trimmed();
    descriptor.priority = map.value(QStringLiteral("priority"), 0).toInt();
    descriptor.available = map.value(QStringLiteral("available"), true).toBool();
    descriptor.seriesTypes = seriesTypesFromVariant(map.value(QStringLiteral("seriesTypes")));
    descriptor.metadata = map.value(QStringLiteral("metadata")).toMap();
    return descriptor;
}

ChartBackendRegistry::ChartBackendRegistry(QObject *parent)
    : QObject(parent)
{
#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
    ChartBackendDescriptor qtGraphs;
    qtGraphs.id = QStringLiteral("qt-graphs");
    qtGraphs.name = QStringLiteral("Qt Graphs");
    qtGraphs.technology = QStringLiteral("qt-quick");
    qtGraphs.priority = 100;
    qtGraphs.seriesTypes = {
        ChartSeriesType::Line,
        ChartSeriesType::Scatter,
        ChartSeriesType::Bar,
        ChartSeriesType::Area
    };
    qtGraphs.metadata.insert(QStringLiteral("module"), QStringLiteral("QtGraphs"));
    registerBackend(std::move(qtGraphs));
#endif
}

QVariantList ChartBackendRegistry::backends() const
{
    QVariantList result;
    result.reserve(m_backends.size());
    for (const auto &backendValue : m_backends)
        result.push_back(backendValue.toVariantMap());
    return result;
}

bool ChartBackendRegistry::registerBackend(ChartBackendDescriptor descriptor)
{
    descriptor.id = normalizedId(descriptor.id);
    if (!descriptor.isValid())
        return false;
    if (descriptor.name.isEmpty())
        descriptor.name = descriptor.id;

    for (auto &existing : m_backends) {
        if (existing.id != descriptor.id)
            continue;
        existing = std::move(descriptor);
        emit backendsChanged();
        return true;
    }

    m_backends.push_back(std::move(descriptor));
    std::stable_sort(m_backends.begin(), m_backends.end(), [](const auto &left, const auto &right) {
        if (left.priority != right.priority)
            return left.priority > right.priority;
        return left.id < right.id;
    });
    emit backendsChanged();
    return true;
}

bool ChartBackendRegistry::unregisterBackend(const QString &id)
{
    const QString normalized = normalizedId(id);
    for (auto it = m_backends.begin(); it != m_backends.end(); ++it) {
        if (it->id != normalized)
            continue;
        m_backends.erase(it);
        emit backendsChanged();
        return true;
    }
    return false;
}

bool ChartBackendRegistry::registerBackendMap(const QVariantMap &descriptor)
{
    return registerBackend(ChartBackendDescriptor::fromVariantMap(descriptor));
}

bool ChartBackendRegistry::unregisterBackendMap(const QString &id)
{
    return unregisterBackend(id);
}

bool ChartBackendRegistry::contains(const QString &id) const
{
    const QString normalized = normalizedId(id);
    return std::any_of(m_backends.cbegin(), m_backends.cend(), [&normalized](const auto &entry) {
        return entry.id == normalized;
    });
}

QVariantMap ChartBackendRegistry::backend(const QString &id) const
{
    const QString normalized = normalizedId(id);
    for (const auto &entry : m_backends) {
        if (entry.id == normalized)
            return entry.toVariantMap();
    }
    return {};
}

QVariantMap ChartBackendRegistry::resolve(int seriesType, const QString &preferredBackend) const
{
    if (seriesType < static_cast<int>(ChartSeriesType::Line)
        || seriesType > static_cast<int>(ChartSeriesType::Area)) {
        return {};
    }

    const auto type = static_cast<ChartSeriesType>(seriesType);
    const QString preferred = normalizedId(preferredBackend);
    if (!preferred.isEmpty()) {
        for (const auto &entry : m_backends) {
            if (entry.id == preferred && entry.available && entry.supports(type))
                return entry.toVariantMap();
        }
    }

    for (const auto &entry : m_backends) {
        if (entry.available && entry.supports(type))
            return entry.toVariantMap();
    }
    return {};
}

QStringList ChartBackendRegistry::availableBackendIds(int seriesType) const
{
    const bool filterType = seriesType >= static_cast<int>(ChartSeriesType::Line)
        && seriesType <= static_cast<int>(ChartSeriesType::Area);
    const auto type = filterType ? static_cast<ChartSeriesType>(seriesType)
                                 : ChartSeriesType::Line;

    QStringList result;
    for (const auto &entry : m_backends) {
        if (!entry.available)
            continue;
        if (filterType && !entry.supports(type))
            continue;
        result.push_back(entry.id);
    }
    return result;
}

void ChartBackendRegistry::clear()
{
    if (m_backends.isEmpty())
        return;
    m_backends.clear();
    emit backendsChanged();
}

QString ChartBackendRegistry::normalizedId(const QString &id)
{
    return id.trimmed().toLower();
}

} // namespace LfQuick
