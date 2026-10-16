#include <LfQuick/Ui/OverlayCoordinator.h>

#include <LfQuick/Core/OperationHandle.h>

#include <QUuid>

namespace LfQuick {

OverlayCoordinator::OverlayCoordinator(QObject *parent)
    : QObject(parent)
{
}

bool OverlayCoordinator::busy() const
{
    return !m_order.isEmpty();
}

int OverlayCoordinator::busyCount() const
{
    return m_order.size();
}

QString OverlayCoordinator::busyMessage() const
{
    const QString token = topToken();
    return token.isEmpty() ? QString{} : m_entries.value(token).message;
}

double OverlayCoordinator::busyProgress() const
{
    const QString token = topToken();
    return token.isEmpty() ? 0.0 : m_entries.value(token).progress;
}

bool OverlayCoordinator::busyCancelable() const
{
    const QString token = topToken();
    return !token.isEmpty() && m_entries.value(token).cancelable;
}

QString OverlayCoordinator::beginBusy(const QString &message, bool cancelable)
{
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);

    Entry entry;
    entry.message = message;
    entry.cancelable = cancelable;
    m_entries.insert(token, entry);
    m_order.append(token);

    emit busyChanged();
    return token;
}

bool OverlayCoordinator::endBusy(const QString &token)
{
    if (token.isEmpty() || !m_entries.contains(token))
        return false;

    removeEntry(token);
    emit busyChanged();
    return true;
}

QString OverlayCoordinator::bindOperation(OperationHandle *operation)
{
    if (!operation)
        return {};

    const QString token = beginBusy(operation->message(), operation->cancelable());
    auto it = m_entries.find(token);
    it->operation = operation;
    it->progress = operation->progress();

    const QPointer<OperationHandle> guardedOperation = operation;

    it->progressConnection = connect(operation, &OperationHandle::progressChanged, this,
                                     [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].progress = guardedOperation->progress();
        emit busyChanged();
    });

    it->messageConnection = connect(operation, &OperationHandle::messageChanged, this,
                                    [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].message = guardedOperation->message();
        emit busyChanged();
    });

    it->cancelableConnection = connect(operation, &OperationHandle::cancelableChanged, this,
                                       [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].cancelable = guardedOperation->cancelable();
        emit busyChanged();
    });

    it->stateConnection = connect(operation, &OperationHandle::stateChanged, this,
                                   [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;

        const auto state = guardedOperation->state();
        if (state == OperationHandle::State::Succeeded
            || state == OperationHandle::State::Failed
            || state == OperationHandle::State::Canceled) {
            endBusy(token);
        } else {
            emit busyChanged();
        }
    });

    emit busyChanged();
    return token;
}

void OverlayCoordinator::cancelTopBusy()
{
    const QString token = topToken();
    if (token.isEmpty())
        return;

    const auto it = m_entries.constFind(token);
    if (it == m_entries.cend())
        return;

    if (it->operation && it->cancelable) {
        it->operation->cancel();
        return;
    }

    if (it->cancelable)
        emit busyCancelRequested(token);
}

QString OverlayCoordinator::topToken() const
{
    return m_order.isEmpty() ? QString{} : m_order.constLast();
}

void OverlayCoordinator::removeEntry(const QString &token)
{
    auto it = m_entries.find(token);
    if (it == m_entries.end())
        return;

    QObject::disconnect(it->progressConnection);
    QObject::disconnect(it->messageConnection);
    QObject::disconnect(it->cancelableConnection);
    QObject::disconnect(it->stateConnection);

    m_entries.erase(it);
    m_order.removeAll(token);
}

} // namespace LfQuick
