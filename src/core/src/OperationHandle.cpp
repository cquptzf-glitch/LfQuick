
#include <LfQuick/Core/OperationHandle.h>

#include <QtGlobal>
#include <utility>

namespace LfQuick {

OperationHandle::OperationHandle(QObject *parent)
    : QObject(parent)
{
}

void OperationHandle::setState(State state)
{
    if (m_state == state) return;
    m_state = state;
    emit stateChanged();
}

void OperationHandle::setProgress(double progress)
{
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_progress, progress)) return;
    m_progress = progress;
    emit progressChanged();
}

void OperationHandle::setMessage(QString message)
{
    if (m_message == message) return;
    m_message = std::move(message);
    emit messageChanged();
}

void OperationHandle::setCancelable(bool cancelable)
{
    if (m_cancelable == cancelable) return;
    m_cancelable = cancelable;
    emit cancelableChanged();
}

void OperationHandle::cancel()
{
    if (m_state == State::Running && m_cancelable)
        emit cancelRequested();
}

} // namespace LfQuick
