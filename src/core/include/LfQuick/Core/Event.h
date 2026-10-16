#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringView>
#include <QVariant>
#include <QVariantMap>

#include <functional>
#include <memory>

namespace LfQuick {

struct LFQUICKCORE_EXPORT Event
{
    QString topic;
    QVariant payload;
    QVariantMap metadata;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
};

class LFQUICKCORE_EXPORT Subscription
{
public:
    Subscription() = default;
    explicit Subscription(std::function<void()> cancel);
    ~Subscription();

    Subscription(const Subscription &) = delete;
    Subscription &operator=(const Subscription &) = delete;
    Subscription(Subscription &&other) noexcept;
    Subscription &operator=(Subscription &&other) noexcept;

    void reset();
    explicit operator bool() const noexcept { return static_cast<bool>(m_cancel); }

private:
    std::function<void()> m_cancel;
};

class LFQUICKCORE_EXPORT EventDispatcher
{
public:
    using Handler = std::function<void(const Event &)>;

    EventDispatcher();
    ~EventDispatcher();

    EventDispatcher(const EventDispatcher &) = delete;
    EventDispatcher &operator=(const EventDispatcher &) = delete;

    Subscription subscribe(QString topic, Handler handler);
    void publish(Event event) const;
    qsizetype subscriberCount(QStringView topic = {}) const;

private:
    struct State;
    std::shared_ptr<State> m_state;
};

} // namespace LfQuick
