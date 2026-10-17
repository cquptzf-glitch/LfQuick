#include <LfQuick/Diagnostics/ObjectInspectorModel.h>

#include <QJsonDocument>
#include <QMetaProperty>

#include <algorithm>

namespace LfQuick {

ObjectInspectorModel::ObjectInspectorModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ObjectInspectorModel::setTarget(QObject *target)
{
    if (m_target == target)
        return;

    QObject::disconnect(m_destroyedConnection);
    m_destroyedConnection = {};
    m_target = target;

    if (m_target) {
        m_destroyedConnection = connect(m_target, &QObject::destroyed, this, [this] {
            m_target = nullptr;
            m_destroyedConnection = {};
            rebuild();
            emit targetChanged();
        });
    }

    rebuild();
    emit targetChanged();
}

void ObjectInspectorModel::setFilter(const QString &filter)
{
    const QString normalized = filter.trimmed();
    if (m_filter == normalized)
        return;
    m_filter = normalized;
    rebuild();
    emit filterChanged();
}

int ObjectInspectorModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant ObjectInspectorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case NameRole: return entry.name;
    case TypeRole: return entry.type;
    case ValueRole: return entry.value;
    case ValueTextRole: return valueText(entry.value);
    case WritableRole: return entry.writable;
    case DynamicRole: return entry.dynamic;
    default: return {};
    }
}

QHash<int, QByteArray> ObjectInspectorModel::roleNames() const
{
    return {
        {NameRole, QByteArrayLiteral("propertyName")},
        {TypeRole, QByteArrayLiteral("propertyType")},
        {ValueRole, QByteArrayLiteral("propertyValue")},
        {ValueTextRole, QByteArrayLiteral("valueText")},
        {WritableRole, QByteArrayLiteral("writable")},
        {DynamicRole, QByteArrayLiteral("dynamic")}
    };
}

void ObjectInspectorModel::refresh()
{
    rebuild();
}

QVariant ObjectInspectorModel::propertyValue(const QString &name) const
{
    return m_target ? m_target->property(name.trimmed().toUtf8().constData()) : QVariant{};
}

bool ObjectInspectorModel::setPropertyValue(const QString &name, const QVariant &value)
{
    if (!m_target)
        return false;

    const QByteArray propertyName = name.trimmed().toUtf8();
    if (propertyName.isEmpty())
        return false;

    const QMetaObject *meta = m_target->metaObject();
    const int propertyIndex = meta ? meta->indexOfProperty(propertyName.constData()) : -1;
    if (propertyIndex >= 0 && !meta->property(propertyIndex).isWritable())
        return false;

    if (!m_target->setProperty(propertyName.constData(), value))
        return false;

    rebuild();
    return true;
}

QVariantMap ObjectInspectorModel::get(int row) const
{
    if (row < 0 || row >= m_entries.size())
        return {};
    const Entry &entry = m_entries.at(row);
    return {
        {QStringLiteral("name"), entry.name},
        {QStringLiteral("type"), entry.type},
        {QStringLiteral("value"), entry.value},
        {QStringLiteral("valueText"), valueText(entry.value)},
        {QStringLiteral("writable"), entry.writable},
        {QStringLiteral("dynamic"), entry.dynamic}
    };
}

QString ObjectInspectorModel::valueText(const QVariant &value)
{
    if (!value.isValid())
        return QStringLiteral("<invalid>");
    if (value.isNull())
        return QStringLiteral("null");

    if (value.metaType().id() == QMetaType::QVariantMap
        || value.metaType().id() == QMetaType::QVariantList
        || value.metaType().id() == QMetaType::QStringList) {
        const QJsonDocument document = QJsonDocument::fromVariant(value);
        return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
    }

    const QString text = value.toString();
    return text.isEmpty()
        ? QStringLiteral("<%1>").arg(QString::fromLatin1(value.metaType().name()))
        : text;
}

void ObjectInspectorModel::rebuild()
{
    QList<Entry> next;
    if (m_target) {
        const QMetaObject *meta = m_target->metaObject();
        if (meta) {
            next.reserve(meta->propertyCount() + m_target->dynamicPropertyNames().size());
            for (int i = 0; i < meta->propertyCount(); ++i) {
                const QMetaProperty property = meta->property(i);
                const QString name = QString::fromLatin1(property.name());
                if (!m_filter.isEmpty()
                    && !name.contains(m_filter, Qt::CaseInsensitive)
                    && !QString::fromLatin1(property.typeName()).contains(m_filter, Qt::CaseInsensitive)) {
                    continue;
                }

                Entry entry;
                entry.name = name;
                entry.type = QString::fromLatin1(property.typeName());
                entry.value = property.read(m_target);
                entry.writable = property.isWritable();
                next.push_back(std::move(entry));
            }
        }

        for (const QByteArray &nameValue : m_target->dynamicPropertyNames()) {
            const QString name = QString::fromUtf8(nameValue);
            if (!m_filter.isEmpty() && !name.contains(m_filter, Qt::CaseInsensitive))
                continue;

            Entry entry;
            entry.name = name;
            entry.type = QStringLiteral("dynamic");
            entry.value = m_target->property(nameValue.constData());
            entry.writable = true;
            entry.dynamic = true;
            next.push_back(std::move(entry));
        }
    }

    std::stable_sort(next.begin(), next.end(), [](const Entry &left, const Entry &right) {
        if (left.dynamic != right.dynamic)
            return !left.dynamic;
        return left.name.compare(right.name, Qt::CaseInsensitive) < 0;
    });

    beginResetModel();
    const bool countChangedValue = next.size() != m_entries.size();
    m_entries = std::move(next);
    endResetModel();
    if (countChangedValue)
        emit countChanged();
}

} // namespace LfQuick
