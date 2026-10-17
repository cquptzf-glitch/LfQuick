#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/PermissionEvaluator.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QtQml/qqml.h>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT PermissionBinding : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
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
    QML_ANONYMOUS
    Q_PROPERTY(PermissionBinding *permission READ permission CONSTANT FINAL)
    Q_PROPERTY(QVariantMap validationRule READ validationRule WRITE setValidationRule NOTIFY validationRuleChanged FINAL)
    Q_PROPERTY(int focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY focusPolicyChanged FINAL)
    Q_PROPERTY(int focusOrder READ focusOrder WRITE setFocusOrder NOTIFY focusOrderChanged FINAL)
    Q_PROPERTY(bool acceptScanner READ acceptScanner WRITE setAcceptScanner NOTIFY acceptScannerChanged FINAL)
    Q_PROPERTY(QString helpContext READ helpContext WRITE setHelpContext NOTIFY helpContextChanged FINAL)
    Q_PROPERTY(QString reloadKey READ reloadKey WRITE setReloadKey NOTIFY reloadKeyChanged FINAL)
    Q_PROPERTY(QStringList stateProperties READ stateProperties WRITE setStateProperties NOTIFY statePropertiesChanged FINAL)

public:
    explicit LfAttached(QObject *target);

    PermissionBinding *permission() const noexcept { return m_permission; }

    QVariantMap validationRule() const { return m_validationRule; }
    void setValidationRule(const QVariantMap &rule);

    int focusPolicy() const noexcept { return m_focusPolicy; }
    void setFocusPolicy(int policy);

    int focusOrder() const noexcept { return m_focusOrder; }
    void setFocusOrder(int order);

    bool acceptScanner() const noexcept { return m_acceptScanner; }
    void setAcceptScanner(bool value);

    QString helpContext() const { return m_helpContext; }
    void setHelpContext(const QString &context);

    QString reloadKey() const { return m_reloadKey; }
    void setReloadKey(const QString &key);

    QStringList stateProperties() const { return m_stateProperties; }
    void setStateProperties(const QStringList &properties);

signals:
    void validationRuleChanged();
    void focusPolicyChanged();
    void focusOrderChanged();
    void acceptScannerChanged();
    void helpContextChanged();
    void reloadKeyChanged();
    void statePropertiesChanged();

private:
    PermissionBinding *m_permission = nullptr;
    QVariantMap m_validationRule;
    int m_focusPolicy = 0;
    int m_focusOrder = 0;
    bool m_acceptScanner = false;
    QString m_helpContext;
    QString m_reloadKey;
    QStringList m_stateProperties;
};

class LFQUICKUI_EXPORT Lf : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Lf)
    QML_UNCREATABLE("Lf is an attached context namespace")
    QML_ATTACHED(LfAttached)

public:
    enum FocusPolicy {
        Auto = 0,
        Native = 1,
        Explicit = 2,
        Geometric = 3,
        Grid = 4
    };
    Q_ENUM(FocusPolicy)

    static LfAttached *qmlAttachedProperties(QObject *object);
};

} // namespace LfQuick

QML_DECLARE_TYPEINFO(LfQuick::Lf, QML_HAS_ATTACHED_PROPERTIES)
