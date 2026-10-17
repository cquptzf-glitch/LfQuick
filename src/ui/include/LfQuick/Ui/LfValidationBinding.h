#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LfAttached;
class ValidationEvaluator;

class LFQUICKUI_EXPORT LfValidationBinding final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(QVariantMap rule READ rule NOTIFY ruleChanged FINAL)
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged FINAL)
    Q_PROPERTY(bool valid READ valid NOTIFY resultChanged FINAL)
    Q_PROPERTY(QString message READ message NOTIFY resultChanged FINAL)

public:
    explicit LfValidationBinding(QObject *parent = nullptr);

    QObject *target() const noexcept { return m_target; }
    void setTarget(QObject *target);

    QVariant value() const { return m_value; }
    void setValue(const QVariant &value);

    QVariantMap rule() const { return m_rule; }
    QVariantMap result() const { return m_result; }
    bool valid() const { return m_result.value(QStringLiteral("valid"), true).toBool(); }
    QString message() const { return m_result.value(QStringLiteral("message")).toString(); }

    Q_INVOKABLE QVariantMap validateNow();

signals:
    void targetChanged();
    void valueChanged();
    void ruleChanged();
    void resultChanged();

private:
    void reconnectTarget();
    void refreshRule();
    void evaluate();

    QPointer<QObject> m_target;
    QPointer<LfAttached> m_attached;
    ValidationEvaluator *m_evaluator = nullptr;
    QVariant m_value;
    QVariantMap m_rule;
    QVariantMap m_result{{QStringLiteral("valid"), true},
                         {QStringLiteral("message"), QString{}},
                         {QStringLiteral("ruleId"), QString{}}};
    QMetaObject::Connection m_ruleConnection;
    QMetaObject::Connection m_targetDestroyedConnection;
};

} // namespace LfQuick
