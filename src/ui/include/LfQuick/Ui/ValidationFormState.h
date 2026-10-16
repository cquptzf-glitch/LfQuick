#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/ValidationRuleTable.h>

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class ValidationFormState;

class LFQUICKUI_EXPORT ValidationFieldState : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ValidationFieldState)
    QML_UNCREATABLE("ValidationFieldState instances are managed by ValidationFormState")
    Q_PROPERTY(QString fieldId READ fieldId CONSTANT FINAL)
    Q_PROPERTY(QString ruleId READ ruleId NOTIFY ruleIdChanged FINAL)
    Q_PROPERTY(QVariant value READ value NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool touched READ touched NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool valid READ valid NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool hasError READ hasError NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString message READ message NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString externalError READ externalError NOTIFY stateChanged FINAL)

public:
    QString fieldId() const { return m_fieldId; }
    QString ruleId() const { return m_ruleId; }
    QVariant value() const { return m_value; }
    bool touched() const noexcept { return m_touched; }
    bool valid() const noexcept { return m_ruleValid && m_externalError.isEmpty(); }
    bool hasError() const noexcept { return m_touched && !valid(); }
    QString message() const;
    QString externalError() const { return m_externalError; }

signals:
    void ruleIdChanged();
    void stateChanged();

private:
    friend class ValidationFormState;

    ValidationFieldState(QString fieldId, QString ruleId, QObject *parent = nullptr);
    void setRuleId(const QString &ruleId);
    void applyValidation(const QVariant &value, bool valid, const QString &message);
    void setExternalErrorInternal(const QString &message);
    void resetState();

    QString m_fieldId;
    QString m_ruleId;
    QVariant m_value;
    bool m_touched = false;
    bool m_ruleValid = true;
    QString m_ruleMessage;
    QString m_externalError;
};

class LFQUICKUI_EXPORT ValidationFormState : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ValidationRuleTable *ruleTable READ ruleTable WRITE setRuleTable NOTIFY ruleTableChanged FINAL)
    Q_PROPERTY(bool valid READ valid NOTIFY validityChanged FINAL)
    Q_PROPERTY(bool hasErrors READ hasErrors NOTIFY validityChanged FINAL)
    Q_PROPERTY(int errorCount READ errorCount NOTIFY validityChanged FINAL)
    Q_PROPERTY(int fieldCount READ fieldCount NOTIFY fieldCountChanged FINAL)

public:
    explicit ValidationFormState(QObject *parent = nullptr);

    ValidationRuleTable *ruleTable() const noexcept { return m_ruleTable; }
    void setRuleTable(ValidationRuleTable *ruleTable);

    bool valid() const noexcept { return errorCount() == 0; }
    bool hasErrors() const noexcept { return errorCount() > 0; }
    int errorCount() const noexcept;
    int fieldCount() const noexcept { return m_fields.size(); }

    Q_INVOKABLE ValidationFieldState *field(const QString &fieldId,
                                             const QString &ruleId = {});
    Q_INVOKABLE bool validateField(const QString &fieldId,
                                   const QVariant &value,
                                   const QString &ruleId = {});
    Q_INVOKABLE bool validateAll(const QVariantMap &values);
    Q_INVOKABLE void setExternalError(const QString &fieldId, const QString &message);
    Q_INVOKABLE void clearExternalError(const QString &fieldId);
    Q_INVOKABLE bool removeField(const QString &fieldId);
    Q_INVOKABLE void reset();

signals:
    void ruleTableChanged();
    void validityChanged();
    void fieldCountChanged();
    void fieldAdded(const QString &fieldId);
    void fieldRemoved(const QString &fieldId);

private:
    void reconnectRuleTable();
    void revalidateTouched();

    QPointer<ValidationRuleTable> m_ruleTable;
    QHash<QString, ValidationFieldState *> m_fields;
    QMetaObject::Connection m_rulesConnection;
    QMetaObject::Connection m_ruleTableDestroyedConnection;
};

} // namespace LfQuick
