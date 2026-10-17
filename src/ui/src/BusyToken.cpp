#include <LfQuick/Ui/BusyToken.h>

#include <LfQuick/Ui/OverlayCoordinator.h>

#include <utility>

namespace LfQuick {

BusyToken::BusyToken(QString id, OverlayCoordinator *coordinator, QObject *parent)
    : QObject(parent)
    , m_id(std::move(id))
    , m_coordinator(coordinator)
{
    if (!m_coordinator || m_id.isEmpty() || !m_coordinator->hasBusyToken(m_id)) {
        m_active = false;
        return;
    }

    m_busyChangedConnection = connect(m_coordinator, &OverlayCoordinator::busyChanged,
                                      this, [this] {
        if (m_active && (!m_coordinator || !m_coordinator->hasBusyToken(m_id)))
            markInactive();
    });
    m_coordinatorDestroyedConnection = connect(m_coordinator, &QObject::destroyed,
                                               this, [this] {
        m_coordinator = nullptr;
        markInactive();
    });
}

BusyToken::~BusyToken()
{
    if (!m_active || !m_coordinator)
        return;

    // Do not emit activeChanged/released while QObject destruction is already
    // in progress. Disconnect first so OverlayCoordinator::busyChanged cannot
    // re-enter this object while endBusy() tears the token down.
    m_active = false;
    QObject::disconnect(m_busyChangedConnection);
    QObject::disconnect(m_coordinatorDestroyedConnection);
    m_busyChangedConnection = {};
    m_coordinatorDestroyedConnection = {};

    const QPointer<OverlayCoordinator> coordinator = m_coordinator;
    m_coordinator = nullptr;
    if (coordinator)
        coordinator->endBusy(m_id);
}

bool BusyToken::release()
{
    if (!m_active)
        return false;

    bool ended = true;
    if (m_coordinator)
        ended = m_coordinator->endBusy(m_id);

    if (m_active)
        markInactive();
    return ended;
}

void BusyToken::markInactive()
{
    if (!m_active)
        return;
    m_active = false;
    QObject::disconnect(m_busyChangedConnection);
    QObject::disconnect(m_coordinatorDestroyedConnection);
    m_busyChangedConnection = {};
    m_coordinatorDestroyedConnection = {};
    emit activeChanged();
    emit released();
}

} // namespace LfQuick
