#include <LfQuick/Ui/QmlStateSnapshot.h>

#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/PersistentState.h>

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QtQml/qqml.h>

namespace LfQuick {
namespace {

constexpr auto kVersionKey = "version";
constexpr auto kObjectsKey = "objects";
constexpr auto kAmbiguousKeysKey = "ambiguousKeys";
constexpr int kSnapshotVersion = 1;

struct TreeIndex
{
    QHash<QString, QPointer<QObject>> objects;
    QSet<QString> ambiguous;
    QList<QPointer<PersistentState>> persistentStates;
};

LfAttached *attachedState(QObject *object)
{
    if (!object)
        return nullptr;
    return qobject_cast<LfAttached *>(qmlAttachedPropertiesObject<Lf>(object, false));
}

QString stateKey(QObject *object)
{
    if (!object)
        return {};

    if (auto *persistent = qobject_cast<PersistentState *>(object)) {
        if (!persistent->key().isEmpty())
            return persistent->key();
    }

    if (auto *attached = attachedState(object))
        return attached->reloadKey();
    return {};
}

QString scopedKey(const QString &scope, const QString &key)
{
    return scope.isEmpty() ? key : scope + QLatin1Char('/') + key;
}

void indexTree(QObject *object, const QString &scope, TreeIndex &index)
{
    if (!object)
        return;

    if (auto *persistent = qobject_cast<PersistentState *>(object))
        index.persistentStates.push_back(persistent);

    QString childScope = scope;
    const QString key = stateKey(object);
    if (!key.isEmpty()) {
        const QString fullKey = scopedKey(scope, key);
        childScope = fullKey;

        if (index.ambiguous.contains(fullKey)) {
            // Already known to be ambiguous in this scope.
        } else if (index.objects.contains(fullKey)) {
            index.objects.remove(fullKey);
            index.ambiguous.insert(fullKey);
        } else {
            index.objects.insert(fullKey, object);
        }
    }

    for (QObject *child : object->children())
        indexTree(child, childScope, index);
}

QVariantMap captureProperties(QObject *object)
{
    if (!object)
        return {};

    if (auto *persistent = qobject_cast<PersistentState *>(object))
        return persistent->saveState();

    QVariantMap state;
    auto *attached = attachedState(object);
    if (!attached)
        return state;

    for (const QString &propertyName : attached->stateProperties()) {
        const QByteArray name = propertyName.toUtf8();
        const QVariant value = object->property(name.constData());
        if (value.isValid())
            state.insert(propertyName, value);
    }
    return state;
}

int restoreExplicitProperties(QObject *object,
                              const QVariantMap &state,
                              int &skipped)
{
    auto *attached = attachedState(object);
    if (!attached)
        return 0;

    int restored = 0;
    for (const QString &propertyName : attached->stateProperties()) {
        if (!state.contains(propertyName))
            continue;

        const QByteArray name = propertyName.toUtf8();
        if (!object->setProperty(name.constData(), state.value(propertyName))) {
            ++skipped;
            continue;
        }
        ++restored;
    }
    return restored;
}

QStringList sortedKeys(const QSet<QString> &keys)
{
    QStringList result;
    result.reserve(keys.size());
    for (const QString &key : keys)
        result.push_back(key);
    result.sort();
    return result;
}

} // namespace

QVariantMap QmlStateSnapshot::capture(QObject *root)
{
    QVariantMap snapshot;
    snapshot.insert(QString::fromLatin1(kVersionKey), kSnapshotVersion);

    TreeIndex index;
    indexTree(root, {}, index);

    QVariantMap objects;
    for (auto it = index.objects.cbegin(); it != index.objects.cend(); ++it) {
        if (!it.value())
            continue;
        objects.insert(it.key(), captureProperties(it.value()));
    }

    snapshot.insert(QString::fromLatin1(kObjectsKey), objects);
    snapshot.insert(QString::fromLatin1(kAmbiguousKeysKey), sortedKeys(index.ambiguous));
    return snapshot;
}

QmlStateRestoreStats QmlStateSnapshot::restore(QObject *root,
                                                const QVariantMap &snapshot)
{
    QmlStateRestoreStats stats;
    if (!root)
        return stats;

    TreeIndex index;
    indexTree(root, {}, index);

    QSet<QString> ambiguous = index.ambiguous;
    const QStringList serializedAmbiguous =
        snapshot.value(QString::fromLatin1(kAmbiguousKeysKey)).toStringList();
    for (const QString &key : serializedAmbiguous)
        ambiguous.insert(key);
    stats.ambiguousKeys = sortedKeys(ambiguous);

    const bool supported = snapshot.value(QString::fromLatin1(kVersionKey),
                                          kSnapshotVersion).toInt() == kSnapshotVersion;
    const QVariantMap objects = supported
        ? snapshot.value(QString::fromLatin1(kObjectsKey)).toMap()
        : QVariantMap{};

    QSet<PersistentState *> completedPersistentStates;

    for (auto it = index.objects.cbegin(); it != index.objects.cend(); ++it) {
        QObject *object = it.value();
        if (!object)
            continue;

        auto *persistent = qobject_cast<PersistentState *>(object);
        if (ambiguous.contains(it.key())) {
            if (persistent) {
                persistent->markLoaded();
                completedPersistentStates.insert(persistent);
            }
            continue;
        }

        if (!objects.contains(it.key())) {
            if (persistent) {
                persistent->markLoaded();
                completedPersistentStates.insert(persistent);
            }
            continue;
        }

        ++stats.matchedObjects;
        const QVariantMap objectState = objects.value(it.key()).toMap();
        if (persistent) {
            stats.restoredProperties += persistent->restoreState(objectState, true);
            completedPersistentStates.insert(persistent);
        } else {
            stats.restoredProperties += restoreExplicitProperties(
                object, objectState, stats.skippedProperties);
        }
    }

    // Unkeyed persistent containers and persistent containers under ambiguous
    // scopes cannot be restored, but must still complete their initial-load
    // lifecycle so QML code is never left waiting for a reload callback.
    for (const QPointer<PersistentState> &persistent : index.persistentStates) {
        if (persistent && !completedPersistentStates.contains(persistent))
            persistent->markLoaded();
    }

    return stats;
}

QmlStateRestoreStats QmlStateSnapshot::transfer(QObject *newRoot,
                                                 QObject *oldRoot)
{
    return restore(newRoot, capture(oldRoot));
}

} // namespace LfQuick
