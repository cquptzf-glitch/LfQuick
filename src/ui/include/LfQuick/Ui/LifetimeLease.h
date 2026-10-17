#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT LifetimeLease : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfLifetimeLease)
    Q_PROPERTY(QObject* object READ object WRITE setObject NOTIFY objectChanged FINAL)
    Q_PROPERTY(bool held READ held WRITE setHeld NOTIFY heldChanged FINAL)
    Q_PROPERTY(bool retained READ retained NOTIFY retainedChanged FINAL)

public:
    explicit LifetimeLease(QObject *parent = nullptr);
    ~LifetimeLease() override;

    QObject *object() const noexcept { return m_object; }
    void setObject(QObject *object);

    bool held() const noexcept { return m_held; }
    void setHeld(bool held);

    bool retained() const;

    // Returns true when destruction can proceed immediately. When false,
    // one or more leases retained the object and destruction is deferred.
    static bool requestDestroy(QObject *object);
    static bool isRetained(QObject *object);

signals:
    void objectChanged();
    void heldChanged();
    void retainedChanged();
    void dropRequested();
    void aboutToDestroy();

private:
    void connectState();
    void disconnectState();
    void acquire();
    void release();

    QPointer<QObject> m_object;
    bool m_held = false;
    QMetaObject::Connection m_dropConnection;
    QMetaObject::Connection m_destroyConnection;
    QMetaObject::Connection m_stateConnection;
    QMetaObject::Connection m_objectDestroyedConnection;
};

} // namespace LfQuick
