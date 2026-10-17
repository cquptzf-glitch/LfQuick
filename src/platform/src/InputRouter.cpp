#include <LfQuick/Platform/InputRouter.h>

#include <QEvent>
#include <QKeyEvent>

namespace LfQuick {

InputRouter::InputRouter(QObject *parent)
    : QObject(parent)
{
}

InputRouter::~InputRouter()
{
    uninstall();
    clearTargets();
    clearSources();
}

KeyboardWedgeScanner *InputRouter::scanner() const
{
    return m_scanner;
}

void InputRouter::setScanner(KeyboardWedgeScanner *scanner)
{
    if (m_scanner == scanner)
        return;

    QObject::disconnect(m_scannerDestroyedConnection);
    m_scannerDestroyedConnection = {};

    if (m_scanner) {
        auto *previous = m_scanner.data();
        m_scanner = nullptr;
        unregisterSource(previous);
    }

    m_scanner = scanner;
    if (m_scanner) {
        registerSource(m_scanner);
        m_scannerDestroyedConnection = connect(m_scanner, &QObject::destroyed,
                                               this, [this] {
            m_scanner = nullptr;
            m_scannerDestroyedConnection = {};
            emit scannerChanged();
        });
    }
    emit scannerChanged();
}

bool InputRouter::installed() const
{
    return !m_target.isNull();
}

int InputRouter::targetCount() const noexcept
{
    int count = 0;
    for (const auto &target : m_targets) {
        if (target)
            ++count;
    }
    return count;
}

bool InputRouter::registerSource(IInputSource *source)
{
    if (!source || m_sources.contains(source))
        return source != nullptr;

    SourceConnections connections;
    connections.received = connect(source, &IInputSource::inputReceived,
                                   this, &InputRouter::routeInput);
    connections.rejected = connect(source, &IInputSource::inputRejected,
                                   this, [this, source](const QString &reason,
                                                       const QVariantMap &metadata) {
        emit sourceRejected(source ? source->id() : QString{}, reason, metadata);
    });
    connections.destroyed = connect(source, &QObject::destroyed, this, [this, source] {
        if (m_sources.remove(source))
            emit sourcesChanged();
    });

    m_sources.insert(source, connections);
    emit sourcesChanged();
    return true;
}

bool InputRouter::unregisterSource(IInputSource *source)
{
    if (!source)
        return false;

    const bool wasScanner = m_scanner && m_scanner.data() == source;
    if (wasScanner) {
        QObject::disconnect(m_scannerDestroyedConnection);
        m_scannerDestroyedConnection = {};
        m_scanner = nullptr;
    }

    auto it = m_sources.find(source);
    if (it == m_sources.end()) {
        if (wasScanner)
            emit scannerChanged();
        return false;
    }

    QObject::disconnect(it->received);
    QObject::disconnect(it->rejected);
    QObject::disconnect(it->destroyed);
    m_sources.erase(it);
    emit sourcesChanged();
    if (wasScanner)
        emit scannerChanged();
    return true;
}

void InputRouter::clearSources()
{
    if (m_sources.isEmpty()) {
        if (m_scanner) {
            QObject::disconnect(m_scannerDestroyedConnection);
            m_scannerDestroyedConnection = {};
            m_scanner = nullptr;
            emit scannerChanged();
        }
        return;
    }

    const auto sources = m_sources.keys();
    for (IInputSource *source : sources)
        unregisterSource(source);
}

bool InputRouter::registerSourceObject(QObject *sourceObject)
{
    return registerSource(qobject_cast<IInputSource *>(sourceObject));
}

bool InputRouter::unregisterSourceObject(QObject *sourceObject)
{
    return unregisterSource(qobject_cast<IInputSource *>(sourceObject));
}

void InputRouter::clearSourceObjects()
{
    clearSources();
}

bool InputRouter::registerTarget(QObject *targetObject)
{
    auto *target = qobject_cast<InputTarget *>(targetObject);
    if (!target)
        return false;

    pruneTargets();
    for (const auto &candidate : m_targets) {
        if (candidate == target)
            return true;
    }

    m_targets.push_back(target);
    auto connection = connect(target, &QObject::destroyed, this, [this, target] {
        m_targetDestroyedConnections.remove(target);
        pruneTargets();
        emit targetsChanged();
    });
    m_targetDestroyedConnections.insert(target, connection);
    emit targetsChanged();
    return true;
}

bool InputRouter::unregisterTarget(QObject *targetObject)
{
    auto *target = qobject_cast<InputTarget *>(targetObject);
    if (!target)
        return false;

    bool removed = false;
    for (auto it = m_targets.begin(); it != m_targets.end();) {
        if (!*it || *it == target) {
            removed = removed || (*it == target);
            it = m_targets.erase(it);
        } else {
            ++it;
        }
    }

    auto connectionIt = m_targetDestroyedConnections.find(target);
    if (connectionIt != m_targetDestroyedConnections.end()) {
        QObject::disconnect(connectionIt.value());
        m_targetDestroyedConnections.erase(connectionIt);
    }

    if (removed)
        emit targetsChanged();
    return removed;
}

void InputRouter::clearTargets()
{
    if (m_targets.isEmpty() && m_targetDestroyedConnections.isEmpty())
        return;

    for (const auto &connection : m_targetDestroyedConnections)
        QObject::disconnect(connection);
    m_targetDestroyedConnections.clear();
    m_targets.clear();
    emit targetsChanged();
}

bool InputRouter::installOn(QObject *target)
{
    if (!target)
        return false;

    if (m_target == target)
        return true;

    uninstall();
    m_target = target;
    m_target->installEventFilter(this);
    m_installTargetDestroyedConnection = connect(m_target, &QObject::destroyed,
                                                 this, [this] {
        m_target = nullptr;
        m_installTargetDestroyedConnection = {};
        emit installedChanged();
    });
    emit installedChanged();
    return true;
}

void InputRouter::uninstall()
{
    QObject::disconnect(m_installTargetDestroyedConnection);
    m_installTargetDestroyedConnection = {};

    if (m_target) {
        m_target->removeEventFilter(this);
        m_target.clear();
        emit installedChanged();
    }
}

bool InputRouter::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_scanner || !m_scanner->isEnabled() || event->type() != QEvent::KeyPress)
        return QObject::eventFilter(watched, event);

    if (m_scanner->handleKeyEvent(static_cast<QKeyEvent *>(event)))
        return true;

    return QObject::eventFilter(watched, event);
}

void InputRouter::routeInput(const QString &value, const QVariantMap &metadata)
{
    pruneTargets();

    InputTarget *bestTarget = nullptr;
    for (const auto &candidate : m_targets) {
        if (!candidate || !candidate->accepts(metadata))
            continue;
        if (!bestTarget || candidate->priority() > bestTarget->priority())
            bestTarget = candidate;
    }

    if (!bestTarget) {
        emit inputUnhandled(value, metadata);
        return;
    }

    bestTarget->deliver(value, metadata);
    emit inputRouted(bestTarget->id(), metadata.value(QStringLiteral("sourceId")).toString(), metadata);
}

void InputRouter::pruneTargets()
{
    for (auto it = m_targets.begin(); it != m_targets.end();) {
        if (!*it)
            it = m_targets.erase(it);
        else
            ++it;
    }
}

} // namespace LfQuick
