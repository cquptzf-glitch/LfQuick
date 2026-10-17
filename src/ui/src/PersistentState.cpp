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

QVariantMap PersistentState::saveState() const
{
    QVariantMap state;
    const QMetaObject *meta = metaObject();

    // Persist every property introduced below PersistentState, not only the
    // final metaobject layer. This keeps C++ intermediate state-container
    // classes composable while excluding QObject/PersistentState framework
    // properties such as objectName and key.
    const int firstPersistentProperty = PersistentState::staticMetaObject.propertyCount();
    for (int i = firstPersistentProperty; i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (!property.isReadable())
            continue;
        const QVariant value = property.read(this);
        if (value.isValid())
            state.insert(QString::fromLatin1(property.name()), value);
    }
    return state;
}

int PersistentState::restoreState(const QVariantMap &state, bool wasReloaded)
{
    int restored = 0;
    for (auto it = state.cbegin(); it != state.cend(); ++it) {
        const QByteArray name = it.key().toUtf8();
        const int propertyIndex = metaObject()->indexOfProperty(name.constData());
        if (propertyIndex < 0)
            continue;

        const QMetaProperty property = metaObject()->property(propertyIndex);
        // Serialized snapshots may outlive a refactor. Only properties below
        // the PersistentState base boundary are eligible for restoration so a
        // malicious/stale map cannot mutate framework properties such as key.
        if (propertyIndex < PersistentState::staticMetaObject.propertyCount()
            || !property.isWritable()) {
            continue;
        }
        if (property.write(this, it.value()))
            ++restored;
    }

    emit loaded();
    if (wasReloaded)
        emit reloaded();
    return restored;
}

void PersistentState::markLoaded()
{
    emit loaded();
}

int PersistentState::restoreFrom(QObject *oldInstance)
{
    auto *oldState = qobject_cast<PersistentState *>(oldInstance);
    if (!oldState) {
        markLoaded();
        return 0;
    }
    return restoreState(oldState->saveState(), true);
}

} // namespace LfQuick
