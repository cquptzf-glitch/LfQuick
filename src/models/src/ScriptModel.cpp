#include <LfQuick/Models/ScriptModel.h>

#include <QJSValueIterator>
#include <QMetaType>

#include <algorithm>
#include <iterator>

namespace LfQuick {
namespace {

struct RecursionPair {
    const QJSValue *left = nullptr;
    const QJSValue *right = nullptr;
    const RecursionPair *previous = nullptr;
};

bool structurallyEqual(const QJSValue &left,
                       const QJSValue &right,
                       const RecursionPair *activePair = nullptr)
{
    if (left.strictlyEquals(right))
        return true;

    const QMetaType jsValueType = QMetaType::fromType<QJSValue>();
    const bool leftIsNative = left.toVariant(QJSValue::RetainJSObjects).metaType() != jsValueType;
    const bool rightIsNative = right.toVariant(QJSValue::RetainJSObjects).metaType() != jsValueType;
    if (leftIsNative || rightIsNative)
        return false;

    if (!left.isObject() || !right.isObject()
        || left.isCallable() || right.isCallable()
        || left.isArray() != right.isArray()) {
        return false;
    }

    // Cyclic object graphs are intentionally treated as non-equal unless they
    // were caught by strict identity above. This prevents unbounded recursion.
    for (const RecursionPair *pair = activePair; pair; pair = pair->previous) {
        if (left.strictlyEquals(*pair->left)
            || right.strictlyEquals(*pair->right)) {
            return false;
        }
    }

    const RecursionPair next{&left, &right, activePair};
    QJSValueIterator leftIt(left);
    QJSValueIterator rightIt(right);

    while (leftIt.hasNext()) {
        if (!rightIt.hasNext())
            return false;

        leftIt.next();
        rightIt.next();

        const QString leftName = leftIt.name();
        const QString rightName = rightIt.name();
        const QJSValue leftValue = leftIt.value();

        if (leftName == rightName) {
            if (!structurallyEqual(leftValue, rightIt.value(), &next))
                return false;
        } else {
            if (!right.hasOwnProperty(leftName)
                || !structurallyEqual(leftValue, right.property(leftName), &next)) {
                return false;
            }
        }
    }

    return !rightIt.hasNext();
}

} // namespace

ScriptModel::ScriptModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QList<QJSValue> ScriptModel::values() const
{
    QList<QJSValue> result = m_values;
    if (m_modifying)
        result.detach();
    return result;
}

void ScriptModel::setValues(const QList<QJSValue> &newValues)
{
    if (m_modifying) {
        m_stagedValues = newValues;
        return;
    }

    const int oldCount = m_values.size();
    bool changed = false;

    // The incremental algorithm assumes unique identities. Rather than inherit
    // Quickshell's undefined behavior for duplicate keys, fall back to a model
    // reset. Correctness wins; callers can add keyProperty for stable diffs.
    if (containsDuplicateKeys(m_values) || containsDuplicateKeys(newValues)) {
        bool identical = m_values.size() == newValues.size();
        if (identical) {
            for (int i = 0; i < m_values.size(); ++i) {
                if (!m_values.at(i).strictlyEquals(newValues.at(i))) {
                    identical = false;
                    break;
                }
            }
        }
        if (!identical) {
            resetValues(newValues);
            changed = true;
        }
    } else {
        changed = updateValuesUnique(newValues);
    }

    if (changed) {
        emit valuesChanged();
        if (oldCount != m_values.size())
            emit countChanged();
    }
}

void ScriptModel::setKeyProperty(const QString &property)
{
    const QString normalized = property.trimmed();
    if (m_keyProperty == normalized)
        return;
    m_keyProperty = normalized;
    emit keyPropertyChanged();
}

void ScriptModel::setComparisonMode(ComparisonMode mode)
{
    if (m_comparisonMode == mode)
        return;
    m_comparisonMode = mode;
    emit comparisonModeChanged();
}

int ScriptModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_values.size();
}

QVariant ScriptModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_values.size())
        return {};
    if (role != Qt::UserRole && role != Qt::DisplayRole)
        return {};
    return m_values.at(index.row()).toVariant(QJSValue::RetainJSObjects);
}

QHash<int, QByteArray> ScriptModel::roleNames() const
{
    return {{Qt::UserRole, QByteArrayLiteral("modelData")}};
}

QVariant ScriptModel::get(int index) const
{
    if (index < 0 || index >= m_values.size())
        return {};
    return m_values.at(index).toVariant(QJSValue::RetainJSObjects);
}

bool ScriptModel::valuesEqual(const QJSValue &left, const QJSValue &right) const
{
    if (left.strictlyEquals(right))
        return true;

    QJSValue leftKey = left;
    QJSValue rightKey = right;
    if (!m_keyProperty.isEmpty()) {
        if (left.hasProperty(m_keyProperty))
            leftKey = left.property(m_keyProperty);
        if (right.hasProperty(m_keyProperty))
            rightKey = right.property(m_keyProperty);
    }

    if (m_comparisonMode == ComparisonMode::Identity)
        return leftKey.strictlyEquals(rightKey);
    return structurallyEqual(leftKey, rightKey);
}

bool ScriptModel::containsDuplicateKeys(const QList<QJSValue> &values) const
{
    for (int i = 0; i < values.size(); ++i) {
        for (int j = i + 1; j < values.size(); ++j) {
            if (valuesEqual(values.at(i), values.at(j)))
                return true;
        }
    }
    return false;
}

bool ScriptModel::updateValuesUnique(const QList<QJSValue> &newValues)
{
    bool anyChanges = false;
    m_modifying = true;
    m_values.reserve(newValues.size());

    auto iter = m_values.begin();
    auto newIter = newValues.begin();

    const auto predicateFor = [this](const QJSValue &wanted) {
        return [this, wanted](const QJSValue &candidate) {
            return valuesEqual(candidate, wanted);
        };
    };

    while (true) {
        if (newIter == newValues.end()) {
            if (iter == m_values.end())
                break;

            const int startIndex = static_cast<int>(newValues.size());
            const int endIndex = static_cast<int>(m_values.size() - 1);
            beginRemoveRows({}, startIndex, endIndex);
            m_values.erase(iter, m_values.end());
            endRemoveRows();
            anyChanges = true;
            break;
        }

        if (iter == m_values.end()) {
            const int startIndex = static_cast<int>(m_values.size());
            const int endIndex = static_cast<int>(newValues.size() - 1);
            beginInsertRows({}, startIndex, endIndex);
            m_values.append(newValues.sliced(startIndex));
            endInsertRows();
            anyChanges = true;
            break;
        }

        if (valuesEqual(*newIter, *iter)) {
            ++iter;
            ++newIter;
            continue;
        }

        auto oldIter = std::find_if(iter, m_values.end(), predicateFor(*newIter));
        if (oldIter != m_values.end()) {
            if (std::find_if(newIter, newValues.end(), predicateFor(*iter)) == newValues.end()) {
                auto startIter = iter;
                do {
                    ++iter;
                } while (iter != m_values.end()
                         && std::find_if(newIter, newValues.end(), predicateFor(*iter))
                                == newValues.end());

                const int index = static_cast<int>(std::distance(m_values.begin(), iter));
                const int startIndex = static_cast<int>(std::distance(m_values.begin(), startIter));
                beginRemoveRows({}, startIndex, index - 1);
                iter = m_values.erase(startIter, iter);
                endRemoveRows();
                anyChanges = true;
            } else {
                auto oldStartIter = oldIter;
                auto advancedNewIter = newIter;
                do {
                    ++oldIter;
                    ++advancedNewIter;
                } while (oldIter != m_values.end()
                         && advancedNewIter != newValues.end()
                         && valuesEqual(*oldIter, *advancedNewIter));

                const int index = static_cast<int>(std::distance(m_values.begin(), iter));
                const int oldStartIndex = static_cast<int>(
                    std::distance(m_values.begin(), oldStartIter));
                const int oldEndIndex = static_cast<int>(std::distance(m_values.begin(), oldIter));
                const int length = oldEndIndex - oldStartIndex;

                if (beginMoveRows({}, oldStartIndex, oldEndIndex - 1, {}, index)) {
                    for (int i = 0; i < length; ++i)
                        m_values.move(oldStartIndex + i, index + i);
                    endMoveRows();
                    iter = m_values.begin() + index + length;
                    newIter = advancedNewIter;
                    anyChanges = true;
                } else {
                    // Defensive fallback for an unexpected Qt move constraint.
                    m_modifying = false;
                    resetValues(newValues);
                    return true;
                }
            }
        } else {
            auto startNewIter = newIter;
            do {
                ++newIter;
            } while (newIter != newValues.end()
                     && std::find_if(iter, m_values.end(), predicateFor(*newIter))
                            == m_values.end());

            const int index = static_cast<int>(std::distance(m_values.begin(), iter));
            const int newIndex = static_cast<int>(std::distance(newValues.begin(), newIter));
            const int startNewIndex = static_cast<int>(
                std::distance(newValues.begin(), startNewIter));
            const int length = newIndex - startNewIndex;

            beginInsertRows({}, index, index + length - 1);
            for (int offset = 0; offset < length; ++offset) {
                m_values.insert(index + offset, newValues.at(startNewIndex + offset));
            }
            iter = m_values.begin() + index + length;
            endInsertRows();
            anyChanges = true;
        }
    }

    // Identities may match while the actual JS object/value changed. Replace
    // the payload in place and notify delegates without destroying them.
    for (int i = 0; i < m_values.size();) {
        if (m_values.at(i).strictlyEquals(newValues.at(i))) {
            ++i;
            continue;
        }

        const int first = i;
        do {
            m_values.replace(i, newValues.at(i));
            ++i;
        } while (i < m_values.size()
                 && !m_values.at(i).strictlyEquals(newValues.at(i)));

        emit dataChanged(index(first, 0), index(i - 1, 0), {Qt::UserRole});
        anyChanges = true;
    }

    m_modifying = false;

    if (m_stagedValues.has_value()) {
        const QList<QJSValue> staged = std::move(*m_stagedValues);
        m_stagedValues.reset();
        if (containsDuplicateKeys(m_values) || containsDuplicateKeys(staged)) {
            resetValues(staged);
            anyChanges = true;
        } else {
            anyChanges |= updateValuesUnique(staged);
        }
    }

    return anyChanges;
}

void ScriptModel::resetValues(const QList<QJSValue> &values)
{
    beginResetModel();
    m_values = values;
    endResetModel();
}

} // namespace LfQuick
