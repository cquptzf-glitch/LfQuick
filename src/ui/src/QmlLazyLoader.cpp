#include <LfQuick/Ui/QmlLazyLoader.h>

#include <LfQuick/Ui/LifetimeLease.h>
#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml/qqml.h>

namespace LfQuick {

QmlLazyLoader::QmlLazyLoader(QObject *parent)
    : QObject(parent)
    , m_factory(new QmlObjectFactory(this))
{
}

QmlLazyLoader::~QmlLazyLoader()
{
    cancelHandle();
    QObject::disconnect(m_componentDestroyedConnection);
    QObject::disconnect(m_itemDestroyedConnection);

    if (m_item) {
        QPointer<QObject> item = m_item;
        m_item = nullptr;
        LifetimeLease::requestDestroy(item);
    }
}

QObject *QmlLazyLoader::item()
{
    // Match Quickshell LazyLoader semantics: foreground access is allowed to
    // force an in-flight asynchronous load to completion.
    if (isLoading())
        setActive(true);
    return m_item;
}

bool QmlLazyLoader::isLoading() const noexcept
{
    return m_handle
        && m_handle->status() == QmlObjectLoadHandle::Status::Loading;
}

void QmlLazyLoader::setLoading(bool loading)
{
    if (isActive())
        return;

    if (loading) {
        if (m_targetLoading || isLoading())
            return;
        m_targetLoading = true;
        if (m_complete)
            startAsync();
        return;
    }

    if (!m_targetLoading && !m_handle)
        return;
    m_targetLoading = false;
    cancelHandle();
}

void QmlLazyLoader::setActive(bool active)
{
    if (active) {
        if (isActive()) {
            m_targetActive = true;
            m_targetLoading = false;
            return;
        }

        m_targetActive = true;
        m_targetLoading = false;
        if (!m_complete)
            return;
        startSync();
        return;
    }

    if (!m_targetActive && !m_targetLoading && !m_handle && !m_item)
        return;

    m_targetActive = false;
    m_targetLoading = false;
    cancelHandle();
    unloadItem();
}

void QmlLazyLoader::setActiveAsync(bool active)
{
    if (active) {
        if (isActive() || m_targetLoading || isLoading())
            return;
        setLoading(true);
    } else {
        setActive(false);
    }
}

void QmlLazyLoader::setComponent(QQmlComponent *component)
{
    if (m_component == component)
        return;

    const bool restartActive = m_targetActive || isActive();
    const bool restartLoading = !restartActive && (m_targetLoading || isLoading());

    cancelHandle();
    unloadItem();
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};

    const bool sourceChanged = !m_source.isEmpty();
    m_source = QUrl{};
    m_component = component;
    reconnectComponent();
    setErrorString({});

    emit componentChanged();
    if (sourceChanged)
        emit sourceChanged();
    restartForInputChange(restartActive, restartLoading);
}

void QmlLazyLoader::setSource(const QUrl &source)
{
    if (m_source == source && !m_component)
        return;

    const bool restartActive = m_targetActive || isActive();
    const bool restartLoading = !restartActive && (m_targetLoading || isLoading());

    cancelHandle();
    unloadItem();
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};

    const bool componentChanged = !m_component.isNull();
    m_component = nullptr;
    m_source = source;
    setErrorString({});

    if (componentChanged)
        emit this->componentChanged();
    emit sourceChanged();
    restartForInputChange(restartActive, restartLoading);
}

void QmlLazyLoader::setInitialProperties(const QVariantMap &properties)
{
    if (m_initialProperties == properties)
        return;

    const bool restartActive = m_targetActive || isActive();
    const bool restartLoading = !restartActive && (m_targetLoading || isLoading());

    m_initialProperties = properties;
    emit initialPropertiesChanged();

    // Initial properties are part of object construction (including required
    // QML properties). If they change while a load is active/in flight, the
    // existing object/handle represents stale input and must be recreated.
    if (!m_complete || (!restartActive && !restartLoading))
        return;

    cancelHandle();
    unloadItem();
    restartForInputChange(restartActive, restartLoading);
}

void QmlLazyLoader::classBegin()
{
    m_complete = false;
}

void QmlLazyLoader::componentComplete()
{
    m_complete = true;
    QQmlContext *context = QQmlEngine::contextForObject(this);
    m_factory->setCreationContext(context);
    m_factory->setEngine(context ? context->engine() : qmlEngine(this));

    if (m_targetActive)
        startSync();
    else if (m_targetLoading)
        startAsync();
}

void QmlLazyLoader::restartForInputChange(bool restartActive, bool restartLoading)
{
    m_targetActive = restartActive;
    m_targetLoading = restartLoading;
    if (!m_complete)
        return;
    if (m_targetActive)
        startSync();
    else if (m_targetLoading)
        startAsync();
}

void QmlLazyLoader::reconnectComponent()
{
    if (!m_component)
        return;

    m_componentDestroyedConnection = connect(m_component, &QObject::destroyed, this, [this] {
        m_componentDestroyedConnection = {};
        m_component = nullptr;
        m_targetActive = false;
        m_targetLoading = false;
        cancelHandle();
        unloadItem();
        setErrorString(QStringLiteral("LfLazyLoader component was destroyed"));
        emit componentChanged();
    });
}

bool QmlLazyLoader::hasLoadTarget() const noexcept
{
    return m_component || !m_source.isEmpty();
}

void QmlLazyLoader::startAsync()
{
    if (!m_complete || isActive() || m_handle || !hasLoadTarget())
        return;

    QQmlContext *context = QQmlEngine::contextForObject(this);
    QQmlEngine *loaderEngine = context ? context->engine() : qmlEngine(this);
    QQmlEngine *engine = m_component ? m_component->engine() : loaderEngine;
    if (!engine)
        engine = loaderEngine;

    if (!engine) {
        setErrorString(QStringLiteral("LfLazyLoader requires a QQmlEngine"));
        m_targetLoading = false;
        return;
    }
    if (m_component && loaderEngine && m_component->engine() != loaderEngine) {
        setErrorString(QStringLiteral("LfLazyLoader component belongs to a different QQmlEngine"));
        m_targetLoading = false;
        return;
    }

    m_factory->setEngine(engine);
    m_factory->setCreationContext(context);
    setErrorString({});

    QmlObjectLoadHandle *handle = m_component
        ? m_factory->createAsyncComponent(m_component, m_initialProperties)
        : m_factory->createAsync(resolvedSource(), m_initialProperties);
    m_handle = handle;
    connect(handle, &QmlObjectLoadHandle::finished,
            this, &QmlLazyLoader::finishHandle);

    if (handle->status() == QmlObjectLoadHandle::Status::Loading)
        emit loadingChanged();
    else
        finishHandle();
}

void QmlLazyLoader::startSync()
{
    if (!m_complete || isActive())
        return;

    if (m_handle) {
        m_handle->forceCompletion();
        if (m_handle && m_handle->status() != QmlObjectLoadHandle::Status::Loading)
            finishHandle();
        return;
    }

    if (!hasLoadTarget()) {
        setErrorString(QStringLiteral("LfLazyLoader has no component or source"));
        m_targetActive = false;
        return;
    }

    QQmlContext *context = QQmlEngine::contextForObject(this);
    QQmlEngine *loaderEngine = context ? context->engine() : qmlEngine(this);
    QQmlEngine *engine = m_component ? m_component->engine() : loaderEngine;
    if (!engine) {
        setErrorString(QStringLiteral("LfLazyLoader requires a QQmlEngine"));
        m_targetActive = false;
        return;
    }
    if (m_component && loaderEngine && m_component->engine() != loaderEngine) {
        setErrorString(QStringLiteral("LfLazyLoader component belongs to a different QQmlEngine"));
        m_targetActive = false;
        return;
    }

    m_factory->setEngine(engine);
    m_factory->setCreationContext(context);

    QObject *created = m_component
        ? m_factory->createComponent(m_component, m_initialProperties)
        : m_factory->create(resolvedSource(), m_initialProperties);

    if (!created) {
        setErrorString(m_factory->lastError());
        m_targetActive = false;
        return;
    }

    setErrorString({});
    adoptItem(created);
}

void QmlLazyLoader::finishHandle()
{
    QPointer<QmlObjectLoadHandle> handle = m_handle;
    if (!handle)
        return;

    const auto status = handle->status();
    if (status == QmlObjectLoadHandle::Status::Loading)
        return;

    m_handle = nullptr;
    emit loadingChanged();

    if (status == QmlObjectLoadHandle::Status::Ready) {
        QObject *created = handle->takeObject();
        m_targetLoading = false;
        setErrorString({});
        if (created)
            adoptItem(created);
        else {
            m_targetActive = false;
            setErrorString(QStringLiteral("LfLazyLoader completed without an object"));
        }
    } else if (status == QmlObjectLoadHandle::Status::Error) {
        m_targetLoading = false;
        m_targetActive = false;
        setErrorString(handle->errorString());
    } else {
        m_targetLoading = false;
    }

    handle->deleteLater();
}

void QmlLazyLoader::cancelHandle()
{
    QPointer<QmlObjectLoadHandle> handle = m_handle;
    if (!handle)
        return;

    const bool wasLoading = handle->status() == QmlObjectLoadHandle::Status::Loading;
    m_handle = nullptr;
    disconnect(handle, nullptr, this, nullptr);
    if (wasLoading)
        handle->cancel();
    handle->deleteLater();
    if (wasLoading)
        emit loadingChanged();
}

void QmlLazyLoader::adoptItem(QObject *item)
{
    if (!item)
        return;

    if (m_item && m_item != item)
        unloadItem();

    QObject::disconnect(m_itemDestroyedConnection);
    m_itemDestroyedConnection = {};
    m_item = item;
    item->setParent(this);
    m_targetLoading = false;
    m_targetActive = true;
    m_itemDestroyedConnection = connect(item, &QObject::destroyed, this, [this] {
        m_item = nullptr;
        m_itemDestroyedConnection = {};
        m_targetActive = false;
        m_targetLoading = false;
        emit itemChanged();
        emit activeChanged();
    });
    emit itemChanged();
    emit activeChanged();
}

void QmlLazyLoader::unloadItem()
{
    if (!m_item)
        return;

    QPointer<QObject> oldItem = m_item;
    QObject::disconnect(m_itemDestroyedConnection);
    m_itemDestroyedConnection = {};
    m_item = nullptr;
    emit itemChanged();
    emit activeChanged();
    LifetimeLease::requestDestroy(oldItem);
}

void QmlLazyLoader::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

QUrl QmlLazyLoader::resolvedSource() const
{
    if (auto *context = QQmlEngine::contextForObject(
            const_cast<QmlLazyLoader *>(this))) {
        return context->resolvedUrl(m_source);
    }
    return m_source;
}

} // namespace LfQuick
