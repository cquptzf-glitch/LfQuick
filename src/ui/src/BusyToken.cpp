#include <LfQuick/Ui/BusyToken.h>

#include <LfQuick/Ui/OverlayCoordinator.h>

namespace LfQuick {

BusyToken::BusyToken(QString id, OverlayCoordinator *coordinator, QObject *parent)
    : QObject(parent)
    , m_id(std::move(id))
    , m_coordinator(coordinator)
{
    if (!m_coordinator || m_id.isEmpty()) {
        m_active = false;
        return;
    }

    m_busyEndedConnection = connect(m_coordinator, &OverlayCoordinator::busyEnded,
                                    this, [this](const QString &token) {
        if (token == m_id)
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
    if (m_active && m_coordinator)
        m_coordinator->endBusy(m_id);
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
    QObject::disconnect(m_busyEndedConnection);
    QObject::disconnect(m_coordinatorDestroyedConnection);
    m_busyEndedConnection = {};
    m_coordinatorDestroyedConnection = {};
    emit activeChanged();
    emit released();
}

} // namespace LfQuick
