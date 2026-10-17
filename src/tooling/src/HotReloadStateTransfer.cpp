#include <LfQuick/Tooling/HotReloadStateTransfer.h>

namespace LfQuick {

HotReloadTransferStats HotReloadStateTransfer::transfer(QObject *newRoot,
                                                         QObject *oldRoot)
{
    return QmlStateSnapshot::transfer(newRoot, oldRoot);
}

} // namespace LfQuick
