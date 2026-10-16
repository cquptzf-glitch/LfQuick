#include <LfQuick/Ui/NavigationGuardState.h>

#include <utility>

namespace LfQuick {

NavigationGuardState::NavigationGuardState(QObject *parent)
    : QObject(parent)
{
}

void NavigationGuardState::setDirty(bool value)
{
    if (m_dirty == value)
        return;
    m_dirty = value;
    emit dirtyChanged();
    emit blockingStateChanged();
}

void NavigationGuardState::setBusy(bool value)
{
    if (m_busy == value)
        return;
    m_busy = value;
    emit busyChanged();
    emit blockingStateChanged();
}

void NavigationGuardState::setEnabled(bool value)
{
    if (m_enabled == value)
        return;
    m_enabled = value;
    emit enabledChanged();
    emit blockingStateChanged();
}

void NavigationGuardState::setReason(QString value)
{
    if (m_reason == value)
        return;
    m_reason = std::move(value);
    emit reasonChanged();
    emit blockingStateChanged();
}

void NavigationGuardState::setConfirmRequired(bool value)
{
    if (m_confirmRequired == value)
        return;
    m_confirmRequired = value;
    emit confirmRequiredChanged();
    emit blockingStateChanged();
}

} // namespace LfQuick
