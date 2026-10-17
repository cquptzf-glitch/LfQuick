#include <LfQuick/Tooling/HotReloadStateTransfer.h>

#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/PersistentState.h>

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QtQml/qqml.h>

namespace LfQuick {
namespace {

struct OldIndex
{
    QHash<QString, QPointer<QObject>> objects;
    QSet<QString> ambiguous;
};

LfAttached *attachedState(QObject *object)
{
    if (!object)
        return nullptr;
    return qobject_cast<LfAttached *>(qmlAttachedPropertiesObject<Lf>(object, false));
}

QString reloadKey(QObject *object)
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

void indexOldTree(QObject *object, const QString &scope, OldIndex &index)
{
    if (!object)
        return;

    QString childScope = scope;
    const QString key = reloadKey(object);
    if (!key.isEmpty()) {
        const QString fullKey = scopedKey(scope, key);
        childScope = fullKey;
        if (index.ambiguous.contains(fullKey)) {
            // Keep it ambiguous.
        } else if (index.objects.contains(fullKey)) {
            index.objects.remove(fullKey);
            index.ambiguous.insert(fullKey);
        } else {
            index.objects.insert(fullKey, object);
        }
    }

    for (QObject *child : object->children())
        indexOldTree(child, childScope, index);
}

void restoreExplicitProperties(QObject *object,
                               QObject *oldObject,
                               HotReloadTransferStats &stats)
{
    auto *attached = attachedState(object);
    if (!attached || !oldObject)
        return;

    for (const QString &propertyName : attached->stateProperties()) {
        const QByteArray name = propertyName.toUtf8();
        const QVariant value = oldObject->property(name.constData());
        if (!value.isValid() || !object->setProperty(name.constData(), value)) {
            ++stats.skippedProperties;
            continue;
        }
        ++stats.restoredProperties;
    }
}

void transferNewTree(QObject *object,
                     const QString &scope,
                     const OldIndex &index,
                     HotReloadTransferStats &stats)
{
    if (!object)
        return;

    QString childScope = scope;
    const QString key = reloadKey(object);
    auto *persistent = qobject_cast<PersistentState *>(object);

    if (!key.isEmpty()) {
        const QString fullKey = scopedKey(scope, key);
        childScope = fullKey;

        if (index.ambiguous.contains(fullKey)) {
            if (!stats.ambiguousKeys.contains(fullKey))
                stats.ambiguousKeys.push_back(fullKey);
            if (persistent)
                persistent->restoreFrom(nullptr);
        } else {
            QObject *oldObject = index.objects.value(fullKey);
            if (oldObject) {
                ++stats.matchedObjects;
                if (persistent)
                    stats.restoredProperties += persistent->restoreFrom(oldObject);
                else
                    restoreExplicitProperties(object, oldObject, stats);
            } else if (persistent) {
                persistent->restoreFrom(nullptr);
            }
        }
    } else if (persistent) {
        // A key is required for cross-reload matching, but loaded() still marks
        // completion of the reload stage for an unkeyed state container.
        persistent->restoreFrom(nullptr);
    }

    for (QObject *child : object->children())
        transferNewTree(child, childScope, index, stats);
}

} // namespace

HotReloadTransferStats HotReloadStateTransfer::transfer(QObject *newRoot,
                                                         QObject *oldRoot)
{
    HotReloadTransferStats stats;
    if (!newRoot)
        return stats;

    OldIndex index;
    if (oldRoot)
        indexOldTree(oldRoot, {}, index);
    transferNewTree(newRoot, {}, index, stats);
    stats.ambiguousKeys.sort();
    return stats;
}

} // namespace LfQuick
