#include <LfQuick/Models/ObjectListModel.h>

#include <QSet>

#include <algorithm>
#include <utility>

namespace LfQuick {

ObjectListModel::ObjectListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    if (role != ModelDataRole && role != Qt::DisplayRole)
        return {};
    return QVariant::fromValue(m_entries.at(index.row()).object.data());
}

QHash<int, QByteArray> ObjectListModel::roleNames() const
{
    return {{ModelDataRole, QByteArrayLiteral("modelData")}};
}

QList<QObject *> ObjectListModel::values() const
{
    QList<QObject *> result;
    result.reserve(m_entries.size());
    for (const Entry &entry : m_entries) {
        if (entry.object)
            result.push_back(entry.object);
    }
    return result;
}

QList<QObject *> ObjectListModel::normalize(const QList<QObject *> &values)
{
    QList<QObject *> result;
    QSet<QObject *> seen;
    result.reserve(values.size());
    for (QObject *object : values) {
        if (!object || seen.contains(object))
            continue;
        seen.insert(object);
        result.push_back(object);
    }
    return result;
}

void ObjectListModel::setValues(const QList<QObject *> &rawValues)
{
    const QList<QObject *> target = normalize(rawValues);
    bool changed = false;

    for (int i = m_entries.size() - 1; i >= 0; --i) {
        QObject *object = m_entries.at(i).object;
        if (!object || !target.contains(object)) {
            removeAtInternal(i, false);
            changed = true;
        }
    }

    for (int targetIndex = 0; targetIndex < target.size(); ++targetIndex) {
        QObject *wanted = target.at(targetIndex);
        if (targetIndex < m_entries.size()
            && m_entries.at(targetIndex).object == wanted) {
            continue;
        }

        const int oldIndex = indexOf(wanted);
        if (oldIndex >= 0) {
            QObject *object = m_entries.at(oldIndex).object;
            const int destination = oldIndex < targetIndex ? targetIndex + 1 : targetIndex;
            if (beginMoveRows({}, oldIndex, oldIndex, {}, destination)) {
                m_entries.move(oldIndex, targetIndex);
                endMoveRows();
                emit objectMoved(object, oldIndex, targetIndex);
                changed = true;
            }
        } else if (insertInternal(targetIndex, wanted, false)) {
            changed = true;
        }
    }

    if (changed)
        emit valuesChanged();
}

QObject *ObjectListModel::get(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return nullptr;
    return m_entries.at(index).object;
}

int ObjectListModel::indexOf(QObject *object) const
{
    return indexOfIdentity(object);
}

int ObjectListModel::indexOfIdentity(const QObject *object) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).identity == object)
            return i;
    }
    return -1;
}

bool ObjectListModel::append(QObject *object)
{
    return insert(m_entries.size(), object);
}

bool ObjectListModel::insert(int index, QObject *object)
{
    if (!object || indexOf(object) >= 0)
        return false;
    index = std::clamp(index, 0, m_entries.size());
    return insertInternal(index, object, true);
}

bool ObjectListModel::insertInternal(int index, QObject *object, bool notifyValues)
{
    if (!object)
        return false;

    beginInsertRows({}, index, index);
    Entry entry;
    connectEntry(entry, object);
    m_entries.insert(index, std::move(entry));
    endInsertRows();

    if (notifyValues)
        emit valuesChanged();
    emit countChanged();
    emit objectInserted(object, index);
    return true;
}

bool ObjectListModel::remove(QObject *object)
{
    const int index = indexOf(object);
    return index >= 0 && removeAt(index);
}

bool ObjectListModel::removeAt(int index)
{
    return removeAtInternal(index, true);
}

bool ObjectListModel::removeAtInternal(int index, bool notifyValues)
{
    if (index < 0 || index >= m_entries.size())
        return false;

    QObject *object = m_entries.at(index).object;
    beginRemoveRows({}, index, index);
    QObject::disconnect(m_entries.at(index).destroyedConnection);
    m_entries.removeAt(index);
    endRemoveRows();

    if (notifyValues)
        emit valuesChanged();
    emit countChanged();
    // If this removal was triggered by QObject::destroyed, QPointer is already
    // null. Never surface the stored address identity as a QObject* because it
    // is no longer safe for signal consumers to dereference.
    emit objectRemoved(object, index);
    return true;
}

bool ObjectListModel::move(int from, int to)
{
    if (from < 0 || from >= m_entries.size()
        || to < 0 || to >= m_entries.size()
        || from == to) {
        return false;
    }

    QObject *object = m_entries.at(from).object;
    const int destination = from < to ? to + 1 : to;
    if (!beginMoveRows({}, from, from, {}, destination))
        return false;
    m_entries.move(from, to);
    endMoveRows();

    emit valuesChanged();
    emit objectMoved(object, from, to);
    return true;
}

void ObjectListModel::clear()
{
    if (m_entries.isEmpty())
        return;

    beginResetModel();
    for (const Entry &entry : std::as_const(m_entries))
        QObject::disconnect(entry.destroyedConnection);
    m_entries.clear();
    endResetModel();

    emit valuesChanged();
    emit countChanged();
}

void ObjectListModel::connectEntry(Entry &entry, QObject *object)
{
    entry.identity = object;
    entry.object = object;
    entry.destroyedConnection = connect(object, &QObject::destroyed, this, [this, object] {
        const int index = indexOfIdentity(object);
        if (index >= 0)
            removeAtInternal(index, true);
    });
}

} // namespace LfQuick
