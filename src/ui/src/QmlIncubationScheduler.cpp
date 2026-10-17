#include <LfQuick/Ui/QmlIncubationScheduler.h>

#include <QTimerEvent>

#include <algorithm>

namespace LfQuick {
namespace {

constexpr auto kSchedulerObjectName = "_lfquick_qml_incubation_scheduler";

} // namespace

QmlIncubationScheduler::QmlIncubationScheduler(QObject *parent)
    : QObject(parent)
{
}

QmlIncubationScheduler::~QmlIncubationScheduler()
{
    detachEngine();
}

void QmlIncubationScheduler::setEngine(QQmlEngine *engine)
{
    if (m_engine == engine)
        return;

    const bool wasInstalled = isInstalled();
    detachEngine();
    m_engine = engine;

    if (m_engine) {
        m_engineDestroyedConnection = connect(m_engine, &QObject::destroyed, this, [this] {
            cancelSchedule();
            m_engine = nullptr;
            m_engineDestroyedConnection = {};
            emit engineChanged();
            emit installedChanged();
        });
        install();
    }

    emit engineChanged();
    if (wasInstalled != isInstalled())
        emit installedChanged();
}

void QmlIncubationScheduler::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    if (m_enabled && pendingCount() > 0)
        schedule();
    else if (!m_enabled)
        cancelSchedule();
    emit enabledChanged();
}

void QmlIncubationScheduler::setBudgetMs(int value)
{
    value = std::clamp(value, 1, 50);
    if (m_budgetMs == value)
        return;
    m_budgetMs = value;
    emit budgetChanged();
}

void QmlIncubationScheduler::setIntervalMs(int value)
{
    value = std::clamp(value, 0, 1000);
    if (m_intervalMs == value)
        return;
    m_intervalMs = value;
    if (m_timerId != 0 && m_enabled && pendingCount() > 0) {
        cancelSchedule();
        schedule();
    }
    emit intervalChanged();
}

bool QmlIncubationScheduler::isInstalled() const noexcept
{
    return m_engine && m_engine->incubationController() == this;
}

QmlIncubationScheduler *QmlIncubationScheduler::ensureForEngine(QQmlEngine *engine)
{
    if (!engine)
        return nullptr;

    if (auto *existing = engine->findChild<QmlIncubationScheduler *>(
            QString::fromLatin1(kSchedulerObjectName), Qt::FindDirectChildrenOnly)) {
        return existing;
    }

    // A controller installed by QQuickWindow or by the application is already
    // sufficient to make asynchronous incubators progress. Never replace it.
    if (engine->incubationController())
        return nullptr;

    auto *scheduler = new QmlIncubationScheduler(engine);
    scheduler->setObjectName(QString::fromLatin1(kSchedulerObjectName));
    scheduler->setEngine(engine);
    return scheduler;
}

bool QmlIncubationScheduler::install()
{
    if (!m_engine)
        return false;

    auto *current = m_engine->incubationController();
    if (current == this)
        return true;
    if (current)
        return false;

    m_engine->setIncubationController(this);
    emit installedChanged();
    if (m_enabled && pendingCount() > 0)
        schedule();
    return true;
}

void QmlIncubationScheduler::uninstall()
{
    if (!m_engine || m_engine->incubationController() != this)
        return;
    cancelSchedule();
    m_engine->setIncubationController(nullptr);
    emit installedChanged();
}

void QmlIncubationScheduler::incubateNow(int budgetMs)
{
    if (!m_enabled || pendingCount() <= 0)
        return;

    const int budget = budgetMs > 0 ? std::clamp(budgetMs, 1, 1000) : m_budgetMs;
    incubateFor(budget);
    if (pendingCount() > 0)
        schedule();
}

void QmlIncubationScheduler::timerEvent(QTimerEvent *event)
{
    if (!event || event->timerId() != m_timerId) {
        QObject::timerEvent(event);
        return;
    }

    cancelSchedule();
    incubateNow();
}

void QmlIncubationScheduler::incubatingObjectCountChanged(int)
{
    emit pendingCountChanged();
    if (m_enabled && pendingCount() > 0)
        schedule();
    else if (pendingCount() == 0)
        cancelSchedule();
}

void QmlIncubationScheduler::schedule()
{
    if (!m_enabled || m_timerId != 0 || pendingCount() <= 0)
        return;
    m_timerId = startTimer(m_intervalMs, Qt::PreciseTimer);
}

void QmlIncubationScheduler::cancelSchedule()
{
    if (m_timerId == 0)
        return;
    killTimer(m_timerId);
    m_timerId = 0;
}

void QmlIncubationScheduler::detachEngine()
{
    cancelSchedule();
    QObject::disconnect(m_engineDestroyedConnection);
    m_engineDestroyedConnection = {};

    if (m_engine && m_engine->incubationController() == this)
        m_engine->setIncubationController(nullptr);
    m_engine = nullptr;
}

} // namespace LfQuick
