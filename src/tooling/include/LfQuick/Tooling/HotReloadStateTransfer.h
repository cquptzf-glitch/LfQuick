#pragma once

#include <LfQuick/Tooling/LfQuickToolingGlobal.h>
#include <LfQuick/Ui/QmlStateSnapshot.h>

class QObject;

namespace LfQuick {

using HotReloadTransferStats = QmlStateRestoreStats;

// Compatibility façade retained for Tooling users. The actual state-tree
// algorithm now lives in Ui::QmlStateSnapshot so hot reload, page recreation
// and windows can share the same serialized state semantics.
class LFQUICKTOOLING_EXPORT HotReloadStateTransfer
{
public:
    static HotReloadTransferStats transfer(QObject *newRoot, QObject *oldRoot);
};

} // namespace LfQuick
