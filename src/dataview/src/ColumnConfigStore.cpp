#include <LfQuick/DataView/ColumnConfigStore.h>

#include <QJsonDocument>
#include <QSettings>

#include <memory>

namespace LfQuick {
namespace {

std::unique_ptr<QSettings> createSettings(const QString &filePath)
{
    if (filePath.isEmpty())
        return std::make_unique<QSettings>();
    return std::make_unique<QSettings>(filePath, QSettings::IniFormat);
}

} // namespace

ColumnConfigStore::ColumnConfigStore(QObject *parent)
    : QObject(parent)
{
}

QString ColumnConfigStore::storageNamespace() const
{
    return m_storageNamespace;
}

void ColumnConfigStore::setStorageNamespace(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || normalized == m_storageNamespace)
        return;
    m_storageNamespace = normalized;
    emit storageNamespaceChanged();
}

QString ColumnConfigStore::filePath() const
{
    return m_filePath;
}

void ColumnConfigStore::setFilePath(const QString &value)
{
    if (value == m_filePath)
        return;
    m_filePath = value;
    emit filePathChanged();
}

QString ColumnConfigStore::groupFor(const QString &tableId) const
{
    return m_storageNamespace + QLatin1Char('/') + tableId;
}

bool ColumnConfigStore::save(const QString &tableId,
                             int schemaVersion,
                             const QString &modelSchema,
                             const QVariantList &columns) const
{
    if (tableId.trimmed().isEmpty() || schemaVersion < 1)
        return false;

    auto settings = createSettings(m_filePath);
    settings->beginGroup(groupFor(tableId));
    settings->setValue(QStringLiteral("schemaVersion"), schemaVersion);
    settings->setValue(QStringLiteral("modelSchema"), modelSchema);
    settings->setValue(QStringLiteral("columnsJson"),
                       QJsonDocument::fromVariant(columns).toJson(QJsonDocument::Compact));
    settings->endGroup();
    settings->sync();
    return settings->status() == QSettings::NoError;
}

QVariantMap ColumnConfigStore::load(const QString &tableId,
                                    int schemaVersion,
                                    const QString &modelSchema) const
{
    QVariantMap result{
        {QStringLiteral("matched"), false},
        {QStringLiteral("columns"), QVariantList{}}
    };
    if (tableId.trimmed().isEmpty() || schemaVersion < 1)
        return result;

    auto settings = createSettings(m_filePath);
    settings->beginGroup(groupFor(tableId));
    const int storedVersion = settings->value(QStringLiteral("schemaVersion"), -1).toInt();
    const QString storedModelSchema = settings->value(QStringLiteral("modelSchema")).toString();
    const QByteArray json = settings->value(QStringLiteral("columnsJson")).toByteArray();
    settings->endGroup();

    if (storedVersion != schemaVersion)
        return result;
    if (!modelSchema.isEmpty() && storedModelSchema != modelSchema)
        return result;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isArray())
        return result;

    result.insert(QStringLiteral("matched"), true);
    result.insert(QStringLiteral("columns"), document.toVariant().toList());
    result.insert(QStringLiteral("schemaVersion"), storedVersion);
    result.insert(QStringLiteral("modelSchema"), storedModelSchema);
    return result;
}

void ColumnConfigStore::clear(const QString &tableId) const
{
    if (tableId.trimmed().isEmpty())
        return;
    auto settings = createSettings(m_filePath);
    settings->beginGroup(groupFor(tableId));
    settings->remove(QString{});
    settings->endGroup();
    settings->sync();
}

} // namespace LfQuick
