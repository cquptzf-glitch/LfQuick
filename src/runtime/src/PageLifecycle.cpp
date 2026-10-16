#include <LfQuick/Runtime/PageLifecycle.h>

namespace LfQuick {

PageLifecycle::PageLifecycle(QObject *parent)
    : QObject(parent)
{
}

bool PageLifecycle::canTransition(State from, State to) noexcept
{
    if (from == to)
        return true;

    switch (from) {
    case State::Created:
        return to == State::Loading || to == State::Ready || to == State::Error
            || to == State::Destroying;
    case State::Loading:
        return to == State::Ready || to == State::Error || to == State::Destroying;
    case State::Ready:
        return to == State::Activating || to == State::Destroying || to == State::Error;
    case State::Activating:
        return to == State::Active || to == State::Error || to == State::Destroying;
    case State::Active:
        return to == State::Deactivating || to == State::Error || to == State::Destroying;
    case State::Deactivating:
        return to == State::Ready || to == State::Suspended || to == State::Error
            || to == State::Destroying;
    case State::Suspended:
        return to == State::Activating || to == State::Destroying || to == State::Error;
    case State::Error:
        return to == State::Loading || to == State::Destroying;
    case State::Destroying:
        return to == State::Destroyed;
    case State::Destroyed:
        return false;
    }
    return false;
}

bool PageLifecycle::transitionTo(State nextState)
{
    if (!canTransition(m_state, nextState)) {
        emit invalidTransition(m_state, nextState);
        return false;
    }
    if (m_state == nextState)
        return true;
    m_state = nextState;
    emit stateChanged(m_state);
    return true;
}

} // namespace LfQuick
