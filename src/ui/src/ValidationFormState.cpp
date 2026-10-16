#include <LfQuick/Ui/ValidationFormState.h>

#include <LfQuick/Ui/ValidationRuleTable.h>

#include <utility>

namespace LfQuick {

ValidationFieldState::ValidationFieldState(QString fieldId, QString ruleId, QObject *parent)
    : QObject(parent)
    , m_fieldId(std::move(fieldId))
    , m_ruleId(std::move(ruleId))
{
}

QString ValidationFieldState::message() const
{
    if (!m_externalError.isEmpty())
        return m_externalError;
    return m_ruleMessage;
}

void ValidationFieldState::setRuleId(const QString &ruleId)
{
    const QString normalized = ruleId.trimmed();
    if (m_ruleId == normalized)
        return;
    m_ruleId = normalized;
    emit ruleIdChanged();
    emit stateChanged();
}

void ValidationFieldState::applyValidation(const QVariant &value,
                                           bool valid,
                                           const QString &message)
{
    m_value = value;
    m_touched = true;
    m_ruleValid = valid;
    m_ruleMessage = valid ? QString{} : message;
    emit stateChanged();
}

void ValidationFieldState::setExternalErrorInternal(const QString &message)
{
    const QString normalized = message.trimmed();
    if (m_externalError == normalized && (normalized.isEmpty() || m_touched))
        return;
    m_externalError = normalized;
    if (!normalized.isEmpty())
        m_touched = true;
    emit stateChanged();
}

void ValidationFieldState::resetState()
{
    m_value.clear();
    m_touched = false;
    m_ruleValid = true;
    m_ruleMessage.clear();
    m_externalError.clear();
    emit stateChanged();
}

ValidationFormState::ValidationFormState(QObject *parent)
    : QObject(parent)
{
}

void ValidationFormState::setRuleTable(ValidationRuleTable *ruleTable)
{
    if (m_ruleTable == ruleTable)
        return;

    QObject::disconnect(m_rulesConnection);
    QObject::disconnect(m_ruleTableDestroyedConnection);
    m_rulesConnection = {};
    m_ruleTableDestroyedConnection = {};
    m_ruleTable = ruleTable;
    reconnectRuleTable();
    emit ruleTableChanged();
    revalidateTouched();
}

int ValidationFormState::errorCount() const noexcept
{
    int count = 0;
    for (auto *state : m_fields) {
        if (state && state->hasError())
            ++count;
    }
    return count;
}

ValidationFieldState *ValidationFormState::field(const QString &fieldId,
                                                  const QString &ruleId)
{
    const QString id = fieldId.trimmed();
    if (id.isEmpty())
        return nullptr;

    auto it = m_fields.find(id);
    if (it != m_fields.end()) {
        if (!ruleId.trimmed().isEmpty())
            it.value()->setRuleId(ruleId);
        return it.value();
    }

    const QString effectiveRuleId = ruleId.trimmed().isEmpty() ? id : ruleId.trimmed();
    auto *state = new ValidationFieldState(id, effectiveRuleId, this);
    connect(state, &ValidationFieldState::stateChanged,
            this, &ValidationFormState::validityChanged);
    m_fields.insert(id, state);
    emit fieldCountChanged();
    emit fieldAdded(id);
    emit validityChanged();
    return state;
}

bool ValidationFormState::validateField(const QString &fieldId,
                                        const QVariant &value,
                                        const QString &ruleId)
{
    ValidationFieldState *state = field(fieldId, ruleId);
    if (!state)
        return false;

    if (!m_ruleTable) {
        state->applyValidation(value, false,
                               QStringLiteral("Validation rule table is not configured"));
        return false;
    }

    const QVariantMap result = m_ruleTable->validate(state->ruleId(), value);
    state->applyValidation(value,
                           result.value(QStringLiteral("valid")).toBool(),
                           result.value(QStringLiteral("message")).toString());
    return state->valid();
}

bool ValidationFormState::validateAll(const QVariantMap &values)
{
    bool allValid = true;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        ValidationFieldState *state = field(it.key());
        if (!state || !validateField(it.key(), it.value(), state->ruleId()))
            allValid = false;
    }
    return allValid && valid();
}

void ValidationFormState::setExternalError(const QString &fieldId, const QString &message)
{
    ValidationFieldState *state = field(fieldId);
    if (!state)
        return;
    state->setExternalErrorInternal(message);
}

void ValidationFormState::clearExternalError(const QString &fieldId)
{
    const QString id = fieldId.trimmed();
    const auto it = m_fields.constFind(id);
    if (it == m_fields.cend() || !it.value())
        return;
    it.value()->setExternalErrorInternal({});
}

bool ValidationFormState::removeField(const QString &fieldId)
{
    const QString id = fieldId.trimmed();
    auto it = m_fields.find(id);
    if (it == m_fields.end())
        return false;

    ValidationFieldState *state = it.value();
    m_fields.erase(it);
    if (state)
        state->deleteLater();
    emit fieldCountChanged();
    emit fieldRemoved(id);
    emit validityChanged();
    return true;
}

void ValidationFormState::reset()
{
    for (auto *state : m_fields) {
        if (state)
            state->resetState();
    }
    emit validityChanged();
}

void ValidationFormState::reconnectRuleTable()
{
    if (!m_ruleTable)
        return;

    m_rulesConnection = connect(m_ruleTable, &ValidationRuleTable::rulesChanged,
                                this, &ValidationFormState::revalidateTouched);
    m_ruleTableDestroyedConnection = connect(m_ruleTable, &QObject::destroyed,
                                             this, [this] {
        m_ruleTable = nullptr;
        m_rulesConnection = {};
        m_ruleTableDestroyedConnection = {};
        emit ruleTableChanged();
        revalidateTouched();
    });
}

void ValidationFormState::revalidateTouched()
{
    const auto states = m_fields.values();
    for (auto *state : states) {
        if (!state || !state->touched())
            continue;
        validateField(state->fieldId(), state->value(), state->ruleId());
    }
    emit validityChanged();
}

} // namespace LfQuick
