#pragma once

#include <LfQuick/Integration/LfQuickIntegrationGlobal.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/InputTarget.h>

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LfAttached;

class LFQUICKINTEGRATION_EXPORT LfScannerTarget final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(InputRouter* router READ router WRITE setRouter NOTIFY routerChanged FINAL)
    Q_PROPERTY(QQuickItem* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(QString targetId READ targetId WRITE setTargetId NOTIFY targetIdChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool followActiveFocus READ followActiveFocus WRITE setFollowActiveFocus NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool acceptScanner READ acceptScanner NOTIFY stateChanged FINAL)
    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY stateChanged FINAL)
    Q_PROPERTY(InputTarget* inputTarget READ inputTarget CONSTANT FINAL)

public:
    explicit LfScannerTarget(QObject *parent = nullptr);
    ~LfScannerTarget() override;

    InputRouter *router() const noexcept { return m_router; }
    void setRouter(InputRouter *router);

    QQuickItem *target() const noexcept { return m_target; }
    void setTarget(QQuickItem *target);

    QString targetId() const { return m_targetId; }
    void setTargetId(const QString &targetId);

    bool enabled() const noexcept { return m_enabled; }
    void setEnabled(bool enabled);

    bool followActiveFocus() const noexcept { return m_followActiveFocus; }
    void setFollowActiveFocus(bool follow);

    bool active() const noexcept;
    void setActive(bool active);

    bool acceptScanner() const noexcept;
    int priority() const noexcept { return m_priority; }
    void setPriority(int priority);

    InputTarget *inputTarget() const noexcept { return m_inputTarget; }

signals:
    void routerChanged();
    void targetChanged();
    void targetIdChanged();
    void stateChanged();
    void inputReceived(const QString &value, const QVariantMap &metadata);

private:
    void unregisterFromRouter();
    void registerWithRouter();
    void reconnectTarget();
    void syncInputTarget();

    QPointer<InputRouter> m_router;
    QPointer<QQuickItem> m_target;
    QPointer<LfAttached> m_attached;
    InputTarget *m_inputTarget = nullptr;
    QString m_targetId;
    bool m_enabled = true;
    bool m_followActiveFocus = true;
    bool m_manualActive = false;
    int m_priority = 0;

    QMetaObject::Connection m_routerDestroyedConnection;
    QMetaObject::Connection m_acceptScannerConnection;
    QMetaObject::Connection m_focusConnection;
    QMetaObject::Connection m_targetDestroyedConnection;
};

} // namespace LfQuick
