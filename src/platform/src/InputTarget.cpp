#include <LfQuick/Platform/InputTarget.h>

namespace LfQuick {

InputTarget::InputTarget(QObject *parent)
    : QObject(parent)
{
}

void InputTarget::setId(const QString &id)
{
    const QString normalized = id.trimmed();
    if (m_id == normalized)
        return;
    m_id = normalized;
    emit idChanged();
}

void InputTarget::setTarget(QObject *target)
{
    if (m_target == target)
        return;
    m_target = target;
    emit targetChanged();
}

void InputTarget::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit enabledChanged();
}

void InputTarget::setActive(bool active)
{
    if (m_active == active)
        return;
    m_active = active;
    emit activeChanged();
}

void InputTarget::setAcceptScanner(bool accept)
{
    if (m_acceptScanner == accept)
        return;
    m_acceptScanner = accept;
    emit acceptScannerChanged();
}

void InputTarget::setPriority(int priority)
{
    if (m_priority == priority)
        return;
    m_priority = priority;
    emit priorityChanged();
}

bool InputTarget::accepts(const QVariantMap &metadata) const
{
    if (!m_enabled || !m_active)
        return false;

    const QString kind = metadata.value(QStringLiteral("kind")).toString().trimmed().toLower();
    if (kind == QStringLiteral("scanner"))
        return m_acceptScanner;

    return false;
}

void InputTarget::deliver(const QString &value, const QVariantMap &metadata)
{
    if (!accepts(metadata))
        return;

    if (m_target) {
        bool invoked = QMetaObject::invokeMethod(
            m_target,
            "onLfInputReceived",
            Qt::DirectConnection,
            Q_ARG(QString, value),
            Q_ARG(QVariantMap, metadata));
        Q_UNUSED(invoked);
    }

    emit inputReceived(value, metadata);
}

} // namespace LfQuick
