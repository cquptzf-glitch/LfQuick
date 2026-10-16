#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QAbstractItemModel>
#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT DataEditTransaction : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(bool active READ active NOTIFY stateChanged FINAL)
    Q_PROPERTY(int row READ row NOTIFY stateChanged FINAL)
    Q_PROPERTY(int column READ column NOTIFY stateChanged FINAL)
    Q_PROPERTY(QVariant originalValue READ originalValue NOTIFY stateChanged FINAL)
    Q_PROPERTY(QVariant temporaryValue READ temporaryValue WRITE setTemporaryValue NOTIFY temporaryValueChanged FINAL)
    Q_PROPERTY(ValidationState validationState READ validationState NOTIFY validationChanged FINAL)
    Q_PROPERTY(QString validationMessage READ validationMessage NOTIFY validationChanged FINAL)

public:
    enum class ValidationState {
        NotValidated,
        Valid,
        Invalid
    };
    Q_ENUM(ValidationState)

    explicit DataEditTransaction(QObject *parent = nullptr);

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    bool active() const noexcept { return m_active; }
    int row() const noexcept { return m_row; }
    int column() const noexcept { return m_column; }
    QVariant originalValue() const { return m_originalValue; }
    QVariant temporaryValue() const { return m_temporaryValue; }
    ValidationState validationState() const noexcept { return m_validationState; }
    QString validationMessage() const { return m_validationMessage; }

    Q_INVOKABLE bool begin(int row, int column);
    Q_INVOKABLE void setTemporaryValue(const QVariant &value);
    Q_INVOKABLE bool acceptValidation(bool accepted, const QString &message = {});
    Q_INVOKABLE bool commit();
    Q_INVOKABLE void rollback();
    Q_INVOKABLE void reset();

signals:
    void modelChanged();
    void stateChanged();
    void temporaryValueChanged();
    void validationChanged();
    void editBegan(int row, int column, const QVariant &originalValue);
    void validated(bool accepted, const QString &message);
    void committed(int row, int column, const QVariant &value);
    void rolledBack(int row, int column, const QVariant &originalValue);

private:
    void clearSession();

    QPointer<QAbstractItemModel> m_model;
    bool m_active = false;
    int m_row = -1;
    int m_column = -1;
    QVariant m_originalValue;
    QVariant m_temporaryValue;
    ValidationState m_validationState = ValidationState::NotValidated;
    QString m_validationMessage;
};

} // namespace LfQuick
