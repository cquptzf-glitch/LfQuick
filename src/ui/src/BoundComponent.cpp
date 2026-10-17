#include <LfQuick/Ui/BoundComponent.h>

#include <LfQuick/Ui/LifetimeLease.h>
#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QDebug>
#include <QMetaMethod>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml/qqml.h>

#include <algorithm>
#include <utility>

namespace LfQuick {

BoundComponent::BoundComponent(QQuickItem *parent)
    : QQuickItem(parent)
    , m_factory(new QmlObjectFactory(this))
{
}

BoundComponent::~BoundComponent()
{
    cancelHandle();
    QObject::disconnect(m_componentDestroyedConnection);
    QObject::disconnect(m_itemDestroyedConnection);
    clearBindings();
    clearItemConnections();
    unloadObject();
}

void BoundComponent::setComponent(QQmlComponent *component)
{
    if (m_component == component && m_source.isEmpty())
        return;

    cancelHandle();
    unloadObject();
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};

    const bool sourceChanged = !m_source.isEmpty();
    m_source = {};
    m_component = component;
    reconnectComponent();
    setErrorString({});

    emit componentChanged();
    if (sourceChanged)
        emit sourceChanged();

    if (m_complete && m_component)
        startLoad();
}

void BoundComponent::setSource(const QUrl &source)
{
    if (m_source == source && !m_component)
        return;

    cancelHandle();
    unloadObject();
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};

    const bool componentChanged = !m_component.isNull();
    m_component = nullptr;
    m_source = source;
    setErrorString({});

    if (componentChanged)
        emit this->componentChanged();
    emit sourceChanged();

    if (m_complete && !m_source.isEmpty())
        startLoad();
}

void BoundComponent::setBindValues(bool bindValues)
{
    if (m_bindValues == bindValues)
        return;
    m_bindValues = bindValues;
    emit bindValuesChanged();

    if (!m_item)
        return;
    clearBindings();
    if (m_bindValues)
        bindDynamicProperties();
    bindSignalHandlers();
}

void BoundComponent::setAsynchronous(bool asynchronous)
{
    if (m_asynchronous == asynchronous)
        return;
    m_asynchronous = asynchronous;
    emit asynchronousChanged();
}

bool BoundComponent::loading() const noexcept
{
    return m_handle
        && m_handle->status() == QmlObjectLoadHandle::Status::Loading;
}

void BoundComponent::reload()
{
    if (!m_complete)
        return;
    cancelHandle();
    unloadObject();
    startLoad();
}

void BoundComponent::clear()
{
    cancelHandle();
    unloadObject();
    setErrorString({});
}

void BoundComponent::componentComplete()
{
    QQuickItem::componentComplete();
    m_complete = true;

    QQmlContext *context = QQmlEngine::contextForObject(this);
    m_factory->setCreationContext(context);
    m_factory->setEngine(context ? context->engine() : qmlEngine(this));

    if (m_component || !m_source.isEmpty())
        startLoad();
}

void BoundComponent::forwardDynamicProperty()
{
    if (!m_item)
        return;

    const int signalIndex = senderSignalIndex();
    const auto it = m_propertyBindings.constFind(signalIndex);
    if (it == m_propertyBindings.cend())
        return;

    for (const PropertyBinding &binding : it.value()) {
        if (!binding.source.isReadable() || !binding.target.isWritable())
            continue;
        binding.target.write(m_item, binding.source.read(this));
    }
}

QVariantMap BoundComponent::collectInitialProperties() const
{
    QVariantMap result;
    const QMetaObject *meta = metaObject();
    const int firstDynamicProperty = std::max(
        meta->propertyOffset(), BoundComponent::staticMetaObject.propertyCount());

    for (int i = firstDynamicProperty; i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (property.isReadable())
            result.insert(QString::fromUtf8(property.name()), property.read(this));
    }
    return result;
}

void BoundComponent::startLoad()
{
    if (!m_complete || m_handle || m_item)
        return;
    if (!m_component && m_source.isEmpty()) {
        setErrorString(QStringLiteral("LfBoundComponent has no component or source"));
        return;
    }

    QQmlContext *context = QQmlEngine::contextForObject(this);
    QQmlEngine *loaderEngine = context ? context->engine() : qmlEngine(this);
    QQmlEngine *engine = m_component ? m_component->engine() : loaderEngine;
    if (!engine) {
        setErrorString(QStringLiteral("LfBoundComponent requires a QQmlEngine"));
        emit failed(m_errorString);
        return;
    }
    if (m_component && loaderEngine && m_component->engine() != loaderEngine) {
        setErrorString(QStringLiteral("LfBoundComponent component belongs to a different QQmlEngine"));
        emit failed(m_errorString);
        return;
    }

    m_factory->setEngine(engine);
    m_factory->setCreationContext(context);
    const QVariantMap initialProperties = collectInitialProperties();
    setErrorString({});

    if (!m_asynchronous) {
        QObject *created = m_component
            ? m_factory->createComponent(m_component, initialProperties)
            : m_factory->create(resolvedSource(), initialProperties);
        if (!created) {
            setErrorString(m_factory->lastError());
            emit failed(m_errorString);
            return;
        }
        adoptObject(created);
        return;
    }

    QmlObjectLoadHandle *handle = m_component
        ? m_factory->createAsyncComponent(m_component, initialProperties)
        : m_factory->createAsync(resolvedSource(), initialProperties);
    m_handle = handle;
    connect(handle, &QmlObjectLoadHandle::finished,
            this, &BoundComponent::finishHandle);

    if (handle->status() == QmlObjectLoadHandle::Status::Loading) {
        emit loadingChanged();
    } else {
        finishHandle();
    }
}

void BoundComponent::finishHandle()
{
    QPointer<QmlObjectLoadHandle> handle = m_handle;
    if (!handle || handle->status() == QmlObjectLoadHandle::Status::Loading)
        return;

    const auto status = handle->status();
    m_handle = nullptr;
    emit loadingChanged();

    if (status == QmlObjectLoadHandle::Status::Ready) {
        QObject *created = handle->takeObject();
        if (created) {
            setErrorString({});
            adoptObject(created);
        } else {
            setErrorString(QStringLiteral("LfBoundComponent completed without an object"));
            emit failed(m_errorString);
        }
    } else if (status == QmlObjectLoadHandle::Status::Error) {
        setErrorString(handle->errorString());
        emit failed(m_errorString);
    }

    handle->deleteLater();
}

void BoundComponent::adoptObject(QObject *object)
{
    if (!object)
        return;

    unloadObject();
    m_item = object;
    object->setParent(this);
    m_quickItem = qobject_cast<QQuickItem *>(object);

    QObject::disconnect(m_itemDestroyedConnection);
    m_itemDestroyedConnection = connect(object, &QObject::destroyed, this, [this] {
        m_itemDestroyedConnection = {};
        clearBindings();
        clearItemConnections();
        m_item = nullptr;
        m_quickItem = nullptr;
        setImplicitWidth(0);
        setImplicitHeight(0);
        emit itemChanged();
    });

    if (m_quickItem) {
        m_quickItem->setParentItem(this);
        updateItemSize();
        updateImplicitSize();

        m_itemConnections.push_back(connect(
            this, &QQuickItem::widthChanged, m_quickItem,
            [this] { updateItemSize(); }));
        m_itemConnections.push_back(connect(
            this, &QQuickItem::heightChanged, m_quickItem,
            [this] { updateItemSize(); }));
        m_itemConnections.push_back(connect(
            m_quickItem, &QQuickItem::implicitWidthChanged, this,
            &BoundComponent::updateImplicitSize));
        m_itemConnections.push_back(connect(
            m_quickItem, &QQuickItem::implicitHeightChanged, this,
            &BoundComponent::updateImplicitSize));
    }

    if (m_bindValues)
        bindDynamicProperties();
    bindSignalHandlers();

    emit itemChanged();
    emit loaded();
}

void BoundComponent::bindDynamicProperties()
{
    if (!m_item)
        return;

    const QMetaObject *fromMeta = metaObject();
    const QMetaObject *toMeta = m_item->metaObject();
    const int firstDynamicProperty = std::max(
        fromMeta->propertyOffset(), BoundComponent::staticMetaObject.propertyCount());
    const int forwardingSlotIndex = metaObject()->indexOfSlot("forwardDynamicProperty()");
    const QMetaMethod forwardingSlot = metaObject()->method(forwardingSlotIndex);

    for (int i = firstDynamicProperty; i < fromMeta->propertyCount(); ++i) {
        const QMetaProperty fromProperty = fromMeta->property(i);
        if (!fromProperty.isReadable() || !fromProperty.hasNotifySignal())
            continue;

        const int targetIndex = toMeta->indexOfProperty(fromProperty.name());
        if (targetIndex < 0) {
            qWarning() << "LfBoundComponent property" << fromProperty.name()
                       << "does not exist on loaded object";
            continue;
        }

        const QMetaProperty targetProperty = toMeta->property(targetIndex);
        if (!targetProperty.isWritable()) {
            qWarning() << "LfBoundComponent property" << fromProperty.name()
                       << "is not writable on loaded object";
            continue;
        }

        const int notifyIndex = fromProperty.notifySignalIndex();
        if (!m_propertyBindings.contains(notifyIndex)) {
            const QMetaObject::Connection connection = QObject::connect(
                this, fromProperty.notifySignal(), this, forwardingSlot,
                Qt::AutoConnection);
            if (connection)
                m_bindingConnections.push_back(connection);
        }
        m_propertyBindings[notifyIndex].push_back({fromProperty, targetProperty});
        targetProperty.write(m_item, fromProperty.read(this));
    }
}

void BoundComponent::bindSignalHandlers()
{
    if (!m_item)
        return;

    const QMetaObject *hostMeta = metaObject();
    const QMetaObject *targetMeta = m_item->metaObject();
    const int firstDynamicMethod = std::max(
        hostMeta->methodOffset(), BoundComponent::staticMetaObject.methodCount());

    for (int i = firstDynamicMethod; i < hostMeta->methodCount(); ++i) {
        const QMetaMethod handler = hostMeta->method(i);
        const QByteArray handlerName = handler.name();
        if (!handlerName.startsWith("on") || handlerName.size() <= 2
            || !QChar::fromLatin1(handlerName.at(2)).isUpper()) {
            continue;
        }

        QByteArray signalName = handlerName.mid(2);
        signalName[0] = char(QChar::fromLatin1(signalName.at(0)).toLower().toLatin1());

        QMetaMethod candidate;
        int compatibleCandidates = 0;
        for (int methodIndex = 0; methodIndex < targetMeta->methodCount(); ++methodIndex) {
            const QMetaMethod signal = targetMeta->method(methodIndex);
            if (signal.methodType() != QMetaMethod::Signal || signal.name() != signalName)
                continue;
            if (!QMetaObject::checkConnectArgs(signal, handler))
                continue;
            candidate = signal;
            ++compatibleCandidates;
        }

        if (compatibleCandidates != 1 || !candidate.isValid()) {
            if (compatibleCandidates == 0) {
                qWarning() << "LfBoundComponent handler" << handlerName
                           << "has no compatible signal on loaded object";
            } else {
                qWarning() << "LfBoundComponent handler" << handlerName
                           << "matches multiple overloaded signals";
            }
            continue;
        }

        const QMetaObject::Connection connection = QObject::connect(
            m_item, candidate, this, handler, Qt::AutoConnection);
        if (connection)
            m_bindingConnections.push_back(connection);
    }
}

void BoundComponent::updateItemSize()
{
    if (m_quickItem)
        m_quickItem->setSize(size());
}

void BoundComponent::updateImplicitSize()
{
    if (!m_quickItem)
        return;
    setImplicitWidth(m_quickItem->implicitWidth());
    setImplicitHeight(m_quickItem->implicitHeight());
}

void BoundComponent::unloadObject()
{
    clearBindings();
    if (!m_item) {
        clearItemConnections();
        QObject::disconnect(m_itemDestroyedConnection);
        m_itemDestroyedConnection = {};
        return;
    }

    QPointer<QObject> object = m_item;
    QPointer<QQuickItem> quickItem = m_quickItem;
    QObject::disconnect(m_itemDestroyedConnection);
    m_itemDestroyedConnection = {};
    clearItemConnections();
    m_item = nullptr;
    m_quickItem = nullptr;
    setImplicitWidth(0);
    setImplicitHeight(0);
    emit itemChanged();

    // LifetimeLease::requestDestroy() emits dropRequested synchronously. When a
    // lease retains the old object, LifetimeState detaches QObject ownership but
    // intentionally preserves QQuickItem::parentItem so exit animations can
    // finish in the old visual tree. Only detach immediately when nobody
    // retained the object and physical destruction was scheduled right away.
    const bool destroyingImmediately = LifetimeLease::requestDestroy(object);
    if (destroyingImmediately && quickItem)
        quickItem->setParentItem(nullptr);
}

void BoundComponent::cancelHandle()
{
    QPointer<QmlObjectLoadHandle> handle = m_handle;
    if (!handle)
        return;

    const bool wasLoading = loading();
    m_handle = nullptr;
    disconnect(handle, nullptr, this, nullptr);
    if (handle->status() == QmlObjectLoadHandle::Status::Loading)
        handle->cancel();
    handle->deleteLater();
    if (wasLoading)
        emit loadingChanged();
}

void BoundComponent::reconnectComponent()
{
    if (!m_component)
        return;

    m_componentDestroyedConnection = connect(m_component, &QObject::destroyed, this, [this] {
        m_component = nullptr;
        m_componentDestroyedConnection = {};
        emit componentChanged();
    });
}

void BoundComponent::clearBindings()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_bindingConnections))
        QObject::disconnect(connection);
    m_bindingConnections.clear();
    m_propertyBindings.clear();
}

void BoundComponent::clearItemConnections()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_itemConnections))
        QObject::disconnect(connection);
    m_itemConnections.clear();
}

QUrl BoundComponent::resolvedSource() const
{
    if (auto *context = QQmlEngine::contextForObject(
            const_cast<BoundComponent *>(this))) {
        return context->resolvedUrl(m_source);
    }
    return m_source;
}

void BoundComponent::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

} // namespace LfQuick
