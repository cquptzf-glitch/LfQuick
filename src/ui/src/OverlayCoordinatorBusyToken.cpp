#include <LfQuick/Ui/OverlayCoordinator.h>

#include <LfQuick/Ui/BusyToken.h>

#include <QQmlEngine>

namespace LfQuick {

BusyToken *OverlayCoordinator::acquireBusy(const QString &message, bool cancelable)
{
    auto *token = new BusyToken(beginBusy(message, cancelable), this);
    QQmlEngine::setObjectOwnership(token, QQmlEngine::JavaScriptOwnership);
    return token;
}

bool OverlayCoordinator::hasBusyToken(const QString &token) const
{
    return !token.isEmpty() && m_entries.contains(token);
}

} // namespace LfQuick
