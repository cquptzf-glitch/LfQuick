#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QObject>

namespace LfQuick {

class LFQUICKRUNTIME_EXPORT PageLifecycle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)

public:
    enum class State {
        Created,
        Loading,
        Ready,
        Activating,
        Active,
        Deactivating,
        Suspended,
        Destroying,
        Destroyed,
        Error
    };
    Q_ENUM(State)

    explicit PageLifecycle(QObject *parent = nullptr);

    State state() const noexcept { return m_state; }
    bool transitionTo(State nextState);
    static bool canTransition(State from, State to) noexcept;

signals:
    void stateChanged(LfQuick::PageLifecycle::State state);
    void invalidTransition(LfQuick::PageLifecycle::State from,
                           LfQuick::PageLifecycle::State to);

private:
    State m_state = State::Created;
};

} // namespace LfQuick
