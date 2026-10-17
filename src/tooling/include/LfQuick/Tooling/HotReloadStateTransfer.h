#pragma once

#include <LfQuick/Tooling/LfQuickToolingGlobal.h>

#include <QStringList>

class QObject;

namespace LfQuick {

struct LFQUICKTOOLING_EXPORT HotReloadTransferStats
{
    int matchedObjects = 0;
    int restoredProperties = 0;
    int skippedProperties = 0;
    QStringList ambiguousKeys;
};

class LFQUICKTOOLING_EXPORT HotReloadStateTransfer
{
public:
    // Matches objects by Lf.reloadKey. Keys are scoped by their nearest keyed
    // ancestor, mirroring the reload-scope semantics used by Quickshell.
    // Only properties listed in Lf.stateProperties on the new object are copied.
    static HotReloadTransferStats transfer(QObject *newRoot, QObject *oldRoot);
};

} // namespace LfQuick
