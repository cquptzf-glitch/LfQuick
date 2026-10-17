#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QStringList>
#include <QVariantMap>

class QObject;

namespace LfQuick {

struct LFQUICKUI_EXPORT QmlStateRestoreStats
{
    int matchedObjects = 0;
    int restoredProperties = 0;
    int skippedProperties = 0;
    QStringList ambiguousKeys;
};

// Serializes and restores the explicitly durable parts of a QML object tree.
// Identity is scoped by Lf.reloadKey/PersistentState.key, matching Quickshell's
// reload-scope idea without depending on a specific hot-reload engine.
class LFQUICKUI_EXPORT QmlStateSnapshot
{
public:
    static QVariantMap capture(QObject *root);
    static QmlStateRestoreStats restore(QObject *root, const QVariantMap &snapshot);
    static QmlStateRestoreStats transfer(QObject *newRoot, QObject *oldRoot);
};

} // namespace LfQuick
