#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class OverlayCoordinator;

class LFQUICKUI_EXPORT BusyToken final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("BusyToken instances are created by OverlayCoordinator.acquireBusy()")
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged FINAL)

public:
    BusyToken(QString id, OverlayCoordinator *coordinator, QObject *parent = nullptr);
    ~BusyToken() override;

    QString id() const { return m_id; }
    bool active() const noexcept { return m_active; }

    Q_INVOKABLE bool release();

signals:
    void activeChanged();
    void released();

private:
    void markInactive();

    QString m_id;
    QPointer<OverlayCoordinator> m_coordinator;
    bool m_active = true;
    QMetaObject::Connection m_busyEndedConnection;
    QMetaObject::Connection m_coordinatorDestroyedConnection;
};

} // namespace LfQuick
