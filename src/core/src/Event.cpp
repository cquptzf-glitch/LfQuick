#include <LfQuick/Core/Event.h>

#include <QList>
#include <QMutexLocker>

#include <utility>

namespace LfQuick {

struct EventDispatcher::State
{
    mutable QMutex mutex;
    quint64 nextId = 1;
    QHash<quint64, QPair<QString, Handler>> handlers;
};

Subscription::Subscription(std::function<void()> cancel)
    : m_cancel(std::move(cancel))
{
}

Subscription::~Subscription()
{
    reset();
}

Subscription::Subscription(Subscription &&other) noexcept
    : m_cancel(std::move(other.m_cancel))
{
    other.m_cancel = {};
}

Subscription &Subscription::operator=(Subscription &&other) noexcept
{
    if (this == &other)
        return *this;
    reset();
    m_cancel = std::move(other.m_cancel);
    other.m_cancel = {};
    return *this;
}

void Subscription::reset()
{
    if (!m_cancel)
        return;
    auto cancel = std::move(m_cancel);
    m_cancel = {};
    cancel();
}

EventDispatcher::EventDispatcher()
    : m_state(std::make_shared<State>())
{
}

EventDispatcher::~EventDispatcher() = default;

Subscription EventDispatcher::subscribe(QString topic, Handler handler)
{
    if (!handler)
        return {};

    const auto state = m_state;
    quint64 id = 0;
    {
        QMutexLocker locker(&state->mutex);
        id = state->nextId++;
        state->handlers.insert(id, qMakePair(std::move(topic), std::move(handler)));
    }

    return Subscription([weak = std::weak_ptr<State>(state), id] {
        if (const auto locked = weak.lock()) {
            QMutexLocker locker(&locked->mutex);
            locked->handlers.remove(id);
        }
    });
}

void EventDispatcher::publish(Event event) const
{
    if (event.timestamp.isNull())
        event.timestamp = QDateTime::currentDateTimeUtc();

    QList<Handler> handlers;
    {
        QMutexLocker locker(&m_state->mutex);
        handlers.reserve(m_state->handlers.size());
        for (auto it = m_state->handlers.cbegin(); it != m_state->handlers.cend(); ++it) {
            const auto &registration = it.value();
            if (registration.first.isEmpty() || registration.first == event.topic)
                handlers.push_back(registration.second);
        }
    }

    for (const auto &handler : handlers)
        handler(event);
}

qsizetype EventDispatcher::subscriberCount(QStringView topic) const
{
    QMutexLocker locker(&m_state->mutex);
    if (topic.isEmpty())
        return m_state->handlers.size();

    qsizetype count = 0;
    for (auto it = m_state->handlers.cbegin(); it != m_state->handlers.cend(); ++it) {
        if (it.value().first == topic)
            ++count;
    }
    return count;
}

} // namespace LfQuick
