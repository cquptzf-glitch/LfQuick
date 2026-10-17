#include <LfQuick/Ui/PostReloadHook.h>

#include <QMetaObject>

namespace LfQuick {

PostReloadHook::PostReloadHook(QObject *parent)
    : QObject(parent)
{
}

void PostReloadHook::classBegin()
{
    m_componentComplete = false;
}

void PostReloadHook::componentComplete()
{
    m_componentComplete = true;
    m_initialCompletionPending = true;

    // Managed page/window/hot-reload flows restore state synchronously after
    // component creation. Queueing the fallback gives QmlStateSnapshot a chance
    // to complete the hook with the correct reload flag first, while standalone
    // QML usage still receives a deterministic initial-load callback.
    QMetaObject::invokeMethod(this, [this] {
        if (m_initialCompletionPending)
            completeRestore(false);
    }, Qt::QueuedConnection);
}

void PostReloadHook::onPostReload(bool)
{
}

void PostReloadHook::completeRestore(bool reloaded)
{
    m_initialCompletionPending = false;
    m_completed = true;
    m_lastCompletionWasReload = reloaded;
    ++m_completionCount;

    onPostReload(reloaded);

    if (reloaded)
        emit this->reloaded();
    else
        emit loaded();
    emit restoreCompleted(reloaded);
    emit completionChanged();
}

} // namespace LfQuick
