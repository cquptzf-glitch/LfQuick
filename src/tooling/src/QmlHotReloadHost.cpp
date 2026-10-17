#include <LfQuick/Tooling/QmlHotReloadHost.h>

#include <LfQuick/Tooling/HotReloadStateTransfer.h>
#include <LfQuick/Tooling/QmlHotReloadController.h>
#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QQmlEngine>
#include <QtQml/qqml.h>

#include <utility>

namespace LfQuick {

QmlHotReloadHost::QmlHotReloadHost(QQuickItem *parent)
    : QQuickItem(parent)
{
}

QmlHotReloadHost::~QmlHotReloadHost()
{
    releaseWatch();
}

void QmlHotReloadHost::setController(QmlHotReloadController *controller)
{
    if (m_controller == controller)
        return;

    releaseWatch();
    if (m_controller)
        disconnect(m_controller, nullptr, this, nullptr);

    m_controller = controller;
    if (m_controller) {
        connect(m_controller, &QmlHotReloadController::reloadRequested,
                this, [this](const QStringList &sources, qint64 revision) {
                    if (!m_autoReload || m_source.isEmpty())
                        return;
                    const QString key = m_source.toString(QUrl::FullyEncoded);
                    if (sources.isEmpty() || sources.contains(key))
                        reloadInternal(revision);
                });
    }

    registerWatch();
    emit controllerChanged();
}

void QmlHotReloadHost::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    releaseWatch();
    m_source = source;
    registerWatch();
    emit sourceChanged();

    if (m_complete && !m_source.isEmpty())
        reloadInternal(m_controller ? m_controller->revision() : m_revision + 1);
}

void QmlHotReloadHost::setDevelopmentSource(const QUrl &source)
{
    if (m_developmentSource == source)
        return;

    releaseWatch();
    m_developmentSource = source;
    registerWatch();
    emit developmentSourceChanged();

    if (m_complete && !m_source.isEmpty())
        reloadInternal(m_controller ? m_controller->revision() : m_revision + 1);
}

void QmlHotReloadHost::setInitialProperties(const QVariantMap &properties)
{
    if (m_initialProperties == properties)
        return;
    m_initialProperties = properties;
    emit initialPropertiesChanged();
}

void QmlHotReloadHost::setPreservedProperties(const QStringList &properties)
{
    if (m_preservedProperties == properties)
        return;
    m_preservedProperties = properties;
    emit preservedPropertiesChanged();
}

void QmlHotReloadHost::setAutoReload(bool value)
{
    if (m_autoReload == value)
        return;
    m_autoReload = value;
    emit autoReloadChanged();
}

bool QmlHotReloadHost::reload()
{
    if (m_source.isEmpty()) {
        setLastError(QStringLiteral("Hot reload host source is empty"));
        setStatus(Status::Failed);
        return false;
    }

    if (m_controller && m_controller->isEnabled()) {
        m_controller->requestReload(m_source);
        return true;
    }

    return reloadInternal(m_revision + 1);
}

void QmlHotReloadHost::componentComplete()
{
    QQuickItem::componentComplete();
    m_complete = true;
    registerWatch();
    if (!m_source.isEmpty())
        reloadInternal(m_controller ? m_controller->revision() : 0);
}

void QmlHotReloadHost::geometryChange(const QRectF &newGeometry,
                                      const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (m_item)
        m_item->setSize(newGeometry.size());
}

void QmlHotReloadHost::registerWatch()
{
    if (!m_controller || m_source.isEmpty())
        return;

    if (m_watchController == m_controller
        && m_watchedLogicalSource == m_source
        && m_watchedDevelopmentSource == m_developmentSource) {
        return;
    }

    releaseWatch();
    if (!m_controller->watchSource(m_source, m_developmentSource)) {
        setLastError(m_controller->lastError());
        return;
    }

    m_watchController = m_controller;
    m_watchedLogicalSource = m_source;
    m_watchedDevelopmentSource = m_developmentSource;
    setLastError({});
}

void QmlHotReloadHost::releaseWatch()
{
    if (m_watchController && !m_watchedLogicalSource.isEmpty())
        m_watchController->unwatchSource(m_watchedLogicalSource);

    m_watchController = nullptr;
    m_watchedLogicalSource = {};
    m_watchedDevelopmentSource = {};
}

bool QmlHotReloadHost::reloadInternal(qint64 attemptedRevision)
{
    QQmlEngine *hostEngine = qmlEngine(this);
    QQmlEngine *engine = m_controller && m_controller->engine()
        ? m_controller->engine()
        : hostEngine;

    if (!engine) {
        setLastError(QStringLiteral("Hot reload host requires a QQmlEngine"));
        setStatus(Status::Failed);
        emit reloadFailed(m_lastError, attemptedRevision);
        return false;
    }
    if (hostEngine && hostEngine != engine) {
        setLastError(QStringLiteral("Hot reload controller and host must use the same QQmlEngine"));
        setStatus(Status::Failed);
        emit reloadFailed(m_lastError, attemptedRevision);
        return false;
    }

    const QUrl url = effectiveSource();
    if (!url.isValid() || url.isEmpty()) {
        setLastError(QStringLiteral("Hot reload source is empty or invalid"));
        setStatus(Status::Failed);
        emit reloadFailed(m_lastError, attemptedRevision);
        return false;
    }

    setStatus(Status::Reloading);
    setLastError({});

    QString creationError;
    QObject *created = QmlObjectFactory::createSync(
        engine, url, m_initialProperties, &creationError);
    if (!created) {
        setLastError(creationError.isEmpty()
                         ? QStringLiteral("Hot reload component creation failed")
                         : creationError);
        setStatus(Status::Failed);
        emit reloadFailed(m_lastError, attemptedRevision);
        return false;
    }

    auto *candidate = qobject_cast<QQuickItem *>(created);
    if (!candidate) {
        created->deleteLater();
        setLastError(QStringLiteral("Hot reload component root must inherit QQuickItem"));
        setStatus(Status::Failed);
        emit reloadFailed(m_lastError, attemptedRevision);
        return false;
    }

    const QVariantMap legacyRootState = preservedState();
    QPointer<QQuickItem> previous = m_item;

    candidate->setVisible(false);
    candidate->setParent(this);
    candidate->setParentItem(this);
    candidate->setPosition({0.0, 0.0});
    candidate->setSize(size());

    for (auto it = legacyRootState.cbegin(); it != legacyRootState.cend(); ++it)
        candidate->setProperty(it.key().toUtf8().constData(), it.value());

    // Stable scoped IDs restore state throughout the object tree. The legacy
    // preservedProperties list above remains only as a root-level compatibility
    // path for existing applications. The transfer also completes the initial
    // load stage for PersistentState objects when there is no previous tree.
    HotReloadStateTransfer::transfer(candidate, previous);

    m_item = candidate;
    candidate->setVisible(true);
    emit itemChanged();

    if (previous) {
        previous->setVisible(false);
        previous->setParentItem(nullptr);
        previous->deleteLater();
    }

    if (m_revision != attemptedRevision) {
        m_revision = attemptedRevision;
        emit revisionChanged();
    }
    setLastError({});
    setStatus(Status::Ready);
    emit reloadSucceeded(m_revision);
    return true;
}

QVariantMap QmlHotReloadHost::preservedState() const
{
    QVariantMap state;
    if (!m_item)
        return state;

    for (const QString &name : m_preservedProperties) {
        if (name.isEmpty())
            continue;
        const QVariant value = m_item->property(name.toUtf8().constData());
        if (value.isValid())
            state.insert(name, value);
    }
    return state;
}

void QmlHotReloadHost::setStatus(Status status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

void QmlHotReloadHost::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

QUrl QmlHotReloadHost::effectiveSource() const
{
    if (m_controller)
        return m_controller->resolveSource(m_source);
    return m_developmentSource.isEmpty() ? m_source : m_developmentSource;
}

} // namespace LfQuick
