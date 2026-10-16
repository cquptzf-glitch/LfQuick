#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/PermissionEvaluator.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QtQml/qqml.h>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT PermissionBinding : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PermissionEvaluator *evaluator READ evaluator WRITE setEvaluator NOTIFY evaluatorChanged FINAL)
    Q_PROPERTY(QString permissionId READ permissionId WRITE setPermissionId NOTIFY permissionIdChanged FINAL)
    Q_PROPERTY(QVariantMap context READ context WRITE setContext NOTIFY contextChanged FINAL)
    Q_PROPERTY(bool active READ active NOTIFY stateChanged FINAL)
    Q_PROPERTY(PermissionEvaluator::State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool allowed READ allowed NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool visible READ visible NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool editable READ editable NOTIFY stateChanged FINAL)

public:
    explicit PermissionBinding(QObject *parent = nullptr);

    PermissionEvaluator *evaluator() const noexcept { return m_evaluator; }
    void setEvaluator(PermissionEvaluator *evaluator);

    QString permissionId() const { return m_permissionId; }
    void setPermissionId(const QString &permissionId);

    QVariantMap context() const { return m_context; }
    void setContext(const QVariantMap &context);

    bool active() const noexcept { return !m_permissionId.isEmpty(); }
    PermissionEvaluator::State state() const;
    bool allowed() const;
    bool visible() const;
    bool enabled() const;
    bool editable() const;

signals:
    void evaluatorChanged();
    void permissionIdChanged();
    void contextChanged();
    void stateChanged();

private:
    void reconnectEvaluator();

    QPointer<PermissionEvaluator> m_evaluator;
    QString m_permissionId;
    QVariantMap m_context;
    QMetaObject::Connection m_providerConnection;
    QMetaObject::Connection m_destroyedConnection;
};

class LFQUICKUI_EXPORT LfAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PermissionBinding *permission READ permission CONSTANT FINAL)

public:
    explicit LfAttached(QObject *target);

    PermissionBinding *permission() const noexcept { return m_permission; }

private:
    PermissionBinding *m_permission = nullptr;
};

class LFQUICKUI_EXPORT Lf : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Lf)
    QML_UNCREATABLE("Lf is an attached context namespace")
    QML_ATTACHED(LfAttached)

public:
    static LfAttached *qmlAttachedProperties(QObject *object);
};

} // namespace LfQuick

QML_DECLARE_TYPEINFO(LfQuick::Lf, QML_HAS_ATTACHED_PROPERTIES)
