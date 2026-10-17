#include <LfQuick/Ui/PersistentState.h>

#include <QMetaProperty>

namespace LfQuick {

PersistentState::PersistentState(QObject *parent)
    : QObject(parent)
{
}

void PersistentState::setKey(const QString &key)
{
    const QString normalized = key.trimmed();
    if (m_key == normalized)
        return;
    m_key = normalized;
    emit keyChanged();
}

int PersistentState::restoreFrom(QObject *oldInstance)
{
    auto *oldState = qobject_cast<PersistentState *>(oldInstance);
    if (!oldState) {
        emit loaded();
        return 0;
    }

    int restored = 0;
    const QMetaObject *meta = metaObject();
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (!property.isWritable())
            continue;

        const QVariant oldValue = oldState->property(property.name());
        if (!oldValue.isValid())
            continue;

        if (setProperty(property.name(), oldValue))
            ++restored;
    }

    emit loaded();
    emit reloaded();
    return restored;
}

} // namespace LfQuick
