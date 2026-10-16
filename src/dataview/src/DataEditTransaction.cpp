#include <LfQuick/DataView/DataEditTransaction.h>

namespace LfQuick {

DataEditTransaction::DataEditTransaction(QObject *parent)
    : QObject(parent)
{
}

QAbstractItemModel *DataEditTransaction::model() const
{
    return m_model;
}

void DataEditTransaction::setModel(QAbstractItemModel *model)
{
    if (m_model == model)
        return;
    if (m_active)
        rollback();
    m_model = model;
    emit modelChanged();
}

bool DataEditTransaction::begin(int row, int column)
{
    if (!m_model || row < 0 || column < 0)
        return false;

    const QModelIndex index = m_model->index(row, column);
    if (!index.isValid() || !(m_model->flags(index) & Qt::ItemIsEditable))
        return false;

    if (m_active)
        rollback();

    m_row = row;
    m_column = column;
    m_originalValue = m_model->data(index, Qt::EditRole);
    if (!m_originalValue.isValid())
        m_originalValue = m_model->data(index, Qt::DisplayRole);
    m_temporaryValue = m_originalValue;
    m_validationState = ValidationState::NotValidated;
    m_validationMessage.clear();
    m_active = true;

    emit stateChanged();
    emit temporaryValueChanged();
    emit validationChanged();
    emit editBegan(m_row, m_column, m_originalValue);
    return true;
}

void DataEditTransaction::setTemporaryValue(const QVariant &value)
{
    if (!m_active || m_temporaryValue == value)
        return;
    m_temporaryValue = value;
    m_validationState = ValidationState::NotValidated;
    m_validationMessage.clear();
    emit temporaryValueChanged();
    emit validationChanged();
}

bool DataEditTransaction::acceptValidation(bool accepted, const QString &message)
{
    if (!m_active)
        return false;
    m_validationState = accepted ? ValidationState::Valid : ValidationState::Invalid;
    m_validationMessage = message;
    emit validationChanged();
    emit validated(accepted, message);
    return accepted;
}

bool DataEditTransaction::commit()
{
    if (!m_active || !m_model || m_validationState != ValidationState::Valid)
        return false;

    const QModelIndex index = m_model->index(m_row, m_column);
    if (!index.isValid())
        return false;

    const int committedRow = m_row;
    const int committedColumn = m_column;
    const QVariant committedValue = m_temporaryValue;
    if (!m_model->setData(index, committedValue, Qt::EditRole))
        return false;

    clearSession();
    emit stateChanged();
    emit committed(committedRow, committedColumn, committedValue);
    return true;
}

void DataEditTransaction::rollback()
{
    if (!m_active)
        return;
    const int rolledBackRow = m_row;
    const int rolledBackColumn = m_column;
    const QVariant original = m_originalValue;
    clearSession();
    emit stateChanged();
    emit rolledBack(rolledBackRow, rolledBackColumn, original);
}

void DataEditTransaction::reset()
{
    clearSession();
    emit stateChanged();
}

void DataEditTransaction::clearSession()
{
    m_active = false;
    m_row = -1;
    m_column = -1;
    m_originalValue.clear();
    m_temporaryValue.clear();
    m_validationState = ValidationState::NotValidated;
    m_validationMessage.clear();
    emit temporaryValueChanged();
    emit validationChanged();
}

} // namespace LfQuick
