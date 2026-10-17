#include <LfQuick/Models/ObjectRepeater.h>

#include <QAbstractItemModel>
#include <QJSValue>
#include <QQmlEngine>
#include <QQmlListReference>
#include <QtQml/qqml.h>

#include <algorithm>

namespace LfQuick {
namespace {

QObject *variantObject(const QVariant &value)
{
    if (value.canConvert<QObject *>()) {
        if (QObject *object = value.value<QObject *>())
            return object;
    }

    if (value.metaType() == QMetaType::fromType<QJSValue>()) {
        const QJSValue js = value.value<QJSValue>();
        if (js.isQObject())
            return js.toQObject();
    }
    return nullptr;
}

} // namespace

ObjectRepeater::ObjectRepeater(QObject *parent)
    : QObject(parent)
{
}

ObjectRepeater::~ObjectRepeater()
{
    disconnectModel();
    QObject::disconnect(m_delegateDestroyedConnection);
}

void ObjectRepeater::setModel(const QVariant &model)
{
    if (m_model == model)
        return;

    disconnectModel();
    m_model = model;
    reconnectModel();
    emit modelChanged();
    if (m_complete)
        refresh();
}

void ObjectRepeater::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;

    QObject::disconnect(m_delegateDestroyedConnection);
    m_delegateDestroyedConnection = {};
    clear();
    m_delegate = delegate;

    if (m_delegate) {
        m_delegateDestroyedConnection = connect(m_delegate, &QObject::destroyed, this, [this] {
            m_delegate = nullptr;
            m_delegateDestroyedConnection = {};
            clear();
            setLastError(QStringLiteral("LfObjectRepeater delegate was destroyed"));
            emit delegateChanged();
        });
    }

    emit delegateChanged();
    if (m_complete)
        refresh();
}

void ObjectRepeater::setKeyProperty(const QString &property)
{
    const QString normalized = property.trimmed();
    if (m_keyProperty == normalized)
        return;

    m_keyProperty = normalized;
    emit keyPropertyChanged();
    if (m_complete)
        refresh();
}

QQmlListProperty<QObject> ObjectRepeater::instances()
{
    return QQmlListProperty<QObject>(this, this,
                                     &ObjectRepeater::instanceCount,
                                     &ObjectRepeater::instanceValue);
}

QObject *ObjectRepeater::instanceAt(int index) const
{
    if (index < 0 || index >= m_records.size())
        return nullptr;
    return m_records.at(index).instance;
}

void ObjectRepeater::classBegin()
{
    m_complete = false;
}

void ObjectRepeater::componentComplete()
{
    m_complete = true;
    reconnectModel();
    refresh();
}

qsizetype ObjectRepeater::instanceCount(QQmlListProperty<QObject> *property)
{
    if (!property || !property->object)
        return 0;
    return static_cast<ObjectRepeater *>(property->object)->m_records.size();
}

QObject *ObjectRepeater::instanceValue(QQmlListProperty<QObject> *property, qsizetype index)
{
    if (!property || !property->object)
        return nullptr;
    auto *self = static_cast<ObjectRepeater *>(property->object);
    if (index < 0 || index >= self->m_records.size())
        return nullptr;
    return self->m_records.at(index).instance;
}

QVariantList ObjectRepeater::extractValues() const
{
    if (!m_model.isValid() || m_model.isNull())
        return {};

    if (QObject *object = variantObject(m_model)) {
        if (auto *abstractModel = qobject_cast<QAbstractItemModel *>(object))
            return extractModelRows(abstractModel);
    }

    if (m_model.metaType() == QMetaType::fromType<QJSValue>()) {
        const QJSValue js = m_model.value<QJSValue>();
        if (js.isArray()) {
            QVariantList values;
            const int length = js.property(QStringLiteral("length")).toInt();
            values.reserve(length);
            for (int i = 0; i < length; ++i)
                values.push_back(js.property(i).toVariant(QJSValue::RetainJSObjects));
            return values;
        }
    }

    if (m_model.canConvert<QQmlListReference>()) {
        const QQmlListReference list = m_model.value<QQmlListReference>();
        if (list.isReadable()) {
            QVariantList values;
            values.reserve(list.count());
            for (qsizetype i = 0; i < list.count(); ++i)
                values.push_back(QVariant::fromValue(list.at(i)));
            return values;
        }
    }

    if (m_model.canConvert<QVariantList>())
        return m_model.toList();

    if (m_model.canConvert<QStringList>()) {
        QVariantList values;
        const QStringList strings = m_model.toStringList();
        values.reserve(strings.size());
        for (const QString &value : strings)
            values.push_back(value);
        return values;
    }

    return {};
}

QVariantList ObjectRepeater::extractModelRows(QAbstractItemModel *model) const
{
    QVariantList values;
    if (!model)
        return values;

    const QHash<int, QByteArray> roles = model->roleNames();
    int modelDataRole = -1;
    for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
        if (it.value() == QByteArrayLiteral("modelData")) {
            modelDataRole = it.key();
            break;
        }
    }

    const int rows = model->rowCount();
    values.reserve(rows);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = model->index(row, 0);
        if (modelDataRole >= 0) {
            values.push_back(model->data(index, modelDataRole));
        } else if (roles.isEmpty()) {
            values.push_back(model->data(index, Qt::DisplayRole));
        } else {
            QVariantMap map;
            for (auto role = roles.cbegin(); role != roles.cend(); ++role)
                map.insert(QString::fromUtf8(role.value()), model->data(index, role.key()));
            values.push_back(map);
        }
    }
    return values;
}

QVariant ObjectRepeater::keyFor(const QVariant &value) const
{
    if (m_keyProperty.isEmpty())
        return value;

    if (QObject *object = variantObject(value)) {
        const QVariant property = object->property(m_keyProperty.toUtf8().constData());
        if (property.isValid())
            return property;
    }

    if (value.metaType() == QMetaType::fromType<QJSValue>()) {
        const QJSValue js = value.value<QJSValue>();
        if (js.hasProperty(m_keyProperty))
            return QVariant::fromValue(js.property(m_keyProperty));
    }

    const QVariantMap map = value.toMap();
    if (!map.isEmpty() && map.contains(m_keyProperty))
        return map.value(m_keyProperty);

    // Missing keys should not collapse unrelated values into one identity.
    return value;
}

bool ObjectRepeater::keysEqual(const QVariant &left, const QVariant &right) const
{
    if (left.metaType() == QMetaType::fromType<QJSValue>()
        && right.metaType() == QMetaType::fromType<QJSValue>()) {
        return left.value<QJSValue>().strictlyEquals(right.value<QJSValue>());
    }
    return left == right;
}

int ObjectRepeater::findRecordByKey(const QList<Record> &records,
                                    const QVariant &key) const
{
    for (int i = 0; i < records.size(); ++i) {
        if (keysEqual(records.at(i).key, key))
            return i;
    }
    return -1;
}

QObject *ObjectRepeater::createInstance(const QVariant &modelData, QString *error)
{
    if (error)
        error->clear();
    if (!m_delegate) {
        if (error)
            *error = QStringLiteral("LfObjectRepeater requires a delegate");
        return nullptr;
    }

    QQmlContext *context = QQmlEngine::contextForObject(m_delegate);
    QObject *instance = m_delegate->createWithInitialProperties(
        {{QStringLiteral("modelData"), modelData}}, context);
    if (!instance) {
        if (error)
            *error = m_delegate->errorString().isEmpty()
                ? QStringLiteral("LfObjectRepeater failed to create delegate")
                : m_delegate->errorString();
        return nullptr;
    }

    QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);
    instance->setParent(this);
    return instance;
}

void ObjectRepeater::refresh()
{
    if (!m_complete)
        return;
    if (!m_delegate) {
        if (!m_records.isEmpty())
            clear();
        return;
    }

    setLastError({});
    const QVariantList values = extractValues();

    QList<QObject *> previousInstances;
    previousInstances.reserve(m_records.size());
    for (const Record &record : std::as_const(m_records))
        previousInstances.push_back(record.instance);

    QList<Record> remaining = std::move(m_records);
    m_records.clear();
    QList<Record> next;
    next.reserve(values.size());
    QList<QVariant> seenKeys;
    seenKeys.reserve(values.size());

    for (int modelIndex = 0; modelIndex < values.size(); ++modelIndex) {
        const QVariant modelData = values.at(modelIndex);
        const QVariant key = keyFor(modelData);

        bool duplicate = false;
        for (const QVariant &seen : std::as_const(seenKeys)) {
            if (keysEqual(seen, key)) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            setLastError(QStringLiteral("LfObjectRepeater model contains duplicate keys"));
            continue;
        }
        seenKeys.push_back(key);

        const int oldIndex = findRecordByKey(remaining, key);
        if (oldIndex >= 0) {
            Record record = remaining.takeAt(oldIndex);
            record.key = key;
            record.modelData = modelData;
            if (record.instance) {
                record.instance->setProperty("modelData", modelData);
                record.instance->setProperty("modelIndex", next.size());
                next.push_back(std::move(record));
                continue;
            }
        }

        QString error;
        QObject *instance = createInstance(modelData, &error);
        if (!instance) {
            setLastError(error);
            continue;
        }

        instance->setProperty("modelIndex", next.size());
        Record record;
        record.key = key;
        record.modelData = modelData;
        record.identity = instance;
        record.instance = instance;
        next.push_back(record);

        connect(instance, &QObject::destroyed, this, [this] {
            if (!m_complete)
                return;
            QMetaObject::invokeMethod(this, &ObjectRepeater::refresh, Qt::QueuedConnection);
        });
        emit instanceCreated(instance, next.size() - 1, modelData);
    }

    for (Record &record : remaining) {
        const int previousIndex = previousInstances.indexOf(record.identity);
        if (record.instance)
            emit instanceRemoved(record.instance, previousIndex);
        if (record.instance)
            record.instance->deleteLater();
    }

    const int oldCount = previousInstances.size();
    m_records = std::move(next);

    QList<QObject *> currentInstances;
    currentInstances.reserve(m_records.size());
    for (const Record &record : std::as_const(m_records))
        currentInstances.push_back(record.instance);

    if (previousInstances != currentInstances)
        emit instancesChanged();
    if (oldCount != m_records.size())
        emit countChanged();
}

void ObjectRepeater::clear()
{
    if (m_records.isEmpty())
        return;

    QList<Record> records = std::move(m_records);
    m_records.clear();
    destroyRecords(records);
    emit instancesChanged();
    emit countChanged();
}

void ObjectRepeater::reconnectModel()
{
    disconnectModel();

    QObject *object = variantObject(m_model);
    auto *model = qobject_cast<QAbstractItemModel *>(object);
    if (!model)
        return;

    const auto queueRefresh = [this] {
        if (m_complete)
            refresh();
    };

    m_modelConnections.push_back(connect(model, &QAbstractItemModel::dataChanged,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QAbstractItemModel::rowsInserted,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QAbstractItemModel::rowsRemoved,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QAbstractItemModel::rowsMoved,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QAbstractItemModel::modelReset,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QAbstractItemModel::layoutChanged,
                                         this, queueRefresh));
    m_modelConnections.push_back(connect(model, &QObject::destroyed, this, [this] {
        disconnectModel();
        m_model = {};
        clear();
        emit modelChanged();
    }));
}

void ObjectRepeater::disconnectModel()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_modelConnections))
        QObject::disconnect(connection);
    m_modelConnections.clear();
}

void ObjectRepeater::destroyRecords(QList<Record> &records)
{
    for (int i = 0; i < records.size(); ++i) {
        Record &record = records[i];
        if (record.instance) {
            emit instanceRemoved(record.instance, i);
            record.instance->deleteLater();
        }
    }
    records.clear();
}

void ObjectRepeater::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

} // namespace LfQuick
