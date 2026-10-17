#include <LfQuick/Ui/LifetimeLease.h>

#include <QString>

namespace LfQuick {
namespace {

constexpr auto kLifetimeStateObjectName = "_lfquick_lifetime_state";

class LifetimeState final : public QObject
{
    Q_OBJECT

public:
    explicit LifetimeState(QObject *target)
        : QObject(target)
        , m_target(target)
    {
        setObjectName(QString::fromLatin1(kLifetimeStateObjectName));
    }

    int refs() const noexcept { return m_refs; }
    bool retained() const noexcept { return m_pendingDestroy && m_refs > 0; }

    void retain()
    {
        ++m_refs;
        emit stateChanged();
    }

    void release()
    {
        if (m_refs <= 0)
            return;
        --m_refs;
        emit stateChanged();
        if (m_pendingDestroy && m_refs == 0)
            finishDestroy();
    }

    bool requestDestroy()
    {
        if (!m_target || m_destroyScheduled)
            return true;

        if (!m_pendingDestroy) {
            m_pendingDestroy = true;
            // Give interested leases a synchronous opportunity to retain the
            // object before the destruction decision is made.
            emit dropRequested();
            emit stateChanged();
        }

        if (m_refs == 0) {
            finishDestroy();
            return true;
        }

        // A QObject parent destroys all of its QObject children regardless of
        // deleteLater()/retention state. Once a logical destroy is retained,
        // detach QObject ownership so the lease can actually outlive the owner.
        // QQuickItem::parentItem is intentionally left untouched: an exit
        // animation may still need the old visual tree until the lease drops.
        if (m_target->parent())
            m_target->setParent(nullptr);
        return false;
    }

signals:
    void dropRequested();
    void aboutToDestroy();
    void stateChanged();

private:
    void finishDestroy()
    {
        if (!m_target || m_destroyScheduled)
            return;
        m_destroyScheduled = true;
        emit aboutToDestroy();
        m_target->deleteLater();
    }

    QPointer<QObject> m_target;
    int m_refs = 0;
    bool m_pendingDestroy = false;
    bool m_destroyScheduled = false;
};

LifetimeState *stateFor(QObject *object, bool create)
{
    if (!object)
        return nullptr;

    auto *state = object->findChild<LifetimeState *>(
        QString::fromLatin1(kLifetimeStateObjectName), Qt::FindDirectChildrenOnly);
    if (!state && create)
        state = new LifetimeState(object);
    return state;
}

} // namespace

LifetimeLease::LifetimeLease(QObject *parent)
    : QObject(parent)
{
}

LifetimeLease::~LifetimeLease()
{
    if (m_held)
        release();
}

void LifetimeLease::setObject(QObject *object)
{
    if (m_object == object)
        return;

    const bool wasHeld = m_held;
    if (wasHeld)
        release();
    disconnectState();

    m_object = object;
    connectState();
    if (wasHeld)
        acquire();

    emit objectChanged();
    emit retainedChanged();
}

void LifetimeLease::setHeld(bool held)
{
    if (m_held == held)
        return;

    m_held = held;
    if (m_held)
        acquire();
    else
        release();

    emit heldChanged();
    emit retainedChanged();
}

bool LifetimeLease::retained() const
{
    auto *state = stateFor(m_object, false);
    return state && state->retained();
}

bool LifetimeLease::requestDestroy(QObject *object)
{
    auto *state = stateFor(object, true);
    return !state || state->requestDestroy();
}

bool LifetimeLease::isRetained(QObject *object)
{
    auto *state = stateFor(object, false);
    return state && state->retained();
}

void LifetimeLease::connectState()
{
    if (!m_object)
        return;

    auto *state = stateFor(m_object, true);
    m_dropConnection = connect(state, &LifetimeState::dropRequested,
                               this, &LifetimeLease::dropRequested);
    m_destroyConnection = connect(state, &LifetimeState::aboutToDestroy,
                                  this, &LifetimeLease::aboutToDestroy);
    m_stateConnection = connect(state, &LifetimeState::stateChanged,
                                this, &LifetimeLease::retainedChanged);
    m_objectDestroyedConnection = connect(m_object, &QObject::destroyed, this, [this] {
        disconnectState();
        const bool heldChanged = m_held;
        m_object = nullptr;
        m_held = false;
        emit objectChanged();
        if (heldChanged)
            emit this->heldChanged();
        emit retainedChanged();
    });
}

void LifetimeLease::disconnectState()
{
    QObject::disconnect(m_dropConnection);
    QObject::disconnect(m_destroyConnection);
    QObject::disconnect(m_stateConnection);
    QObject::disconnect(m_objectDestroyedConnection);
    m_dropConnection = {};
    m_destroyConnection = {};
    m_stateConnection = {};
    m_objectDestroyedConnection = {};
}

void LifetimeLease::acquire()
{
    if (auto *state = stateFor(m_object, true))
        state->retain();
}

void LifetimeLease::release()
{
    if (auto *state = stateFor(m_object, false))
        state->release();
}

} // namespace LfQuick

#include "LifetimeLease.moc"
