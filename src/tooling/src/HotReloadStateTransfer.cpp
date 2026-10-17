#include <LfQuick/Tooling/HotReloadStateTransfer.h>

#include <LfQuick/Ui/LfAttached.h>

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

QString scopedKey(const QString &scope, const QString &key)
{
    return scope.isEmpty() ? key : scope + QLatin1Char('/') + key;
}

void indexOldTree(QObject *object, const QString &scope, OldIndex &index)
{
    if (!object)
        return;

    QString childScope = scope;
    if (auto *attached = attachedState(object)) {
        const QString key = attached->reloadKey();
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
    }

    for (QObject *child : object->children())
        indexOldTree(child, childScope, index);
}

void transferNewTree(QObject *object,
                     const QString &scope,
                     const OldIndex &index,
                     HotReloadTransferStats &stats)
{
    if (!object)
        return;

    QString childScope = scope;
    if (auto *attached = attachedState(object)) {
        const QString key = attached->reloadKey();
        if (!key.isEmpty()) {
            const QString fullKey = scopedKey(scope, key);
            childScope = fullKey;

            if (index.ambiguous.contains(fullKey)) {
                if (!stats.ambiguousKeys.contains(fullKey))
                    stats.ambiguousKeys.push_back(fullKey);
            } else {
                QObject *oldObject = index.objects.value(fullKey);
                if (oldObject) {
                    ++stats.matchedObjects;
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
            }
        }
    }

    for (QObject *child : object->children())
        transferNewTree(child, childScope, index, stats);
}

} // namespace

HotReloadTransferStats HotReloadStateTransfer::transfer(QObject *newRoot,
                                                         QObject *oldRoot)
{
    HotReloadTransferStats stats;
    if (!newRoot || !oldRoot)
        return stats;

    OldIndex index;
    indexOldTree(oldRoot, {}, index);
    transferNewTree(newRoot, {}, index, stats);
    stats.ambiguousKeys.sort();
    return stats;
}

} // namespace LfQuick
