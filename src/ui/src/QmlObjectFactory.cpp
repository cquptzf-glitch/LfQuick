#include <LfQuick/Ui/QmlObjectFactory.h>

#include <LfQuick/Ui/QmlIncubationScheduler.h>

#include <QQmlContext>
#include <QQmlIncubator>
#include <QtQml/qqml.h>

#include <utility>

namespace LfQuick {
namespace {

bool contextMatchesEngine(QQmlContext *context, QQmlEngine *engine)
{
    return !context || !engine || context->engine() == engine;
}

} // namespace

class FactoryIncubator final : public QQmlIncubator
{
public:
    explicit FactoryIncubator(QmlObjectLoadHandle *owner)
        : QQmlIncubator(QQmlIncubator::Asynchronous)
        , m_owner(owner)
    {
    }

protected:
    void statusChanged(Status) override
    {
        if (m_owner)
            m_owner->onIncubatorStatusChanged();
    }

private:
    QPointer<QmlObjectLoadHandle> m_owner;
};

QmlObjectLoadHandle::QmlObjectLoadHandle(QUrl source,
                                         QVariantMap initialProperties,
                                         QQmlContext *creationContext,
                                         QObject *parent)
    : QObject(parent)
    , m_source(std::move(source))
    , m_initialProperties(std::move(initialProperties))
    , m_creationContext(creationContext)
{
}

QmlObjectLoadHandle::QmlObjectLoadHandle(QQmlComponent *component,
                                         QVariantMap initialProperties,
                                         QQmlContext *creationContext,
                                         QObject *parent)
    : QObject(parent)
    , m_initialProperties(std::move(initialProperties))
    , m_creationContext(creationContext)
    , m_component(component)
{
    if (!m_creationContext && m_component)
        m_creationContext = m_component->creationContext();
}

QmlObjectLoadHandle::~QmlObjectLoadHandle()
{
    if (m_status == Status::Loading && m_incubator)
        m_incubator->clear();

    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};
    if (m_component)
        disconnect(m_component, nullptr, this, nullptr);
    m_component = nullptr;

    // Disconnect before m_ownedComponent is destroyed. Its QObject::destroyed
    // signal must not re-enter this object while member teardown is underway.
    m_ownedComponent.reset();

    if (m_object)
        m_object->deleteLater();
}

void QmlObjectLoadHandle::begin(QQmlEngine *engine)
{
    if (!engine && m_component)
        engine = m_component->engine();
    m_engine = engine;

    if (!m_engine) {
        fail(QStringLiteral("QML object creation requires a QQmlEngine"));
        return;
    }
    if (!contextMatchesEngine(m_creationContext, m_engine)) {
        fail(QStringLiteral("QML creation context belongs to a different QQmlEngine"));
        return;
    }

    if (!m_component) {
        if (!m_source.isValid() || m_source.isEmpty()) {
            fail(QStringLiteral("QML source URL is empty or invalid"));
            return;
        }

        m_ownedComponent = std::make_unique<QQmlComponent>(
            m_engine, m_source, QQmlComponent::Asynchronous);
        m_component = m_ownedComponent.get();
    }

    if (!m_component) {
        fail(QStringLiteral("QML component is null"));
        return;
    }
    if (m_component->engine() && m_component->engine() != m_engine) {
        fail(QStringLiteral("QML component belongs to a different QQmlEngine"));
        return;
    }
    if (!m_creationContext && m_component->creationContext())
        m_creationContext = m_component->creationContext();
    if (!contextMatchesEngine(m_creationContext, m_engine)) {
        fail(QStringLiteral("QML component creation context belongs to a different QQmlEngine"));
        return;
    }

    m_componentDestroyedConnection = connect(m_component, &QObject::destroyed, this, [this] {
        m_component = nullptr;
        // Externally owned components can disappear while a load is pending.
        // An internally owned component cannot reach this path during normal
        // teardown because the destructor disconnects first.
        m_ownedComponent.release();
        if (m_status == Status::Loading)
            fail(QStringLiteral("QML component was destroyed while loading"));
    });
    connect(m_component, &QQmlComponent::statusChanged,
            this, &QmlObjectLoadHandle::onComponentStatusChanged);
    onComponentStatusChanged();
}

QQmlContext *QmlObjectLoadHandle::effectiveCreationContext() const
{
    if (m_creationContext)
        return m_creationContext;
    if (m_component && m_component->creationContext())
        return m_component->creationContext();
    return nullptr;
}

void QmlObjectLoadHandle::startIncubation()
{
    if (m_status != Status::Loading || !m_component || !m_engine || m_incubator)
        return;

    m_incubator = std::make_unique<FactoryIncubator>(this);
    m_incubator->setInitialProperties(m_initialProperties);
    m_component->create(*m_incubator, effectiveCreationContext());
    onIncubatorStatusChanged();
}

void QmlObjectLoadHandle::onComponentStatusChanged()
{
    if (m_status != Status::Loading || !m_component)
        return;

    switch (m_component->status()) {
    case QQmlComponent::Ready:
        startIncubation();
        break;
    case QQmlComponent::Error:
        fail(m_component->errorString());
        break;
    case QQmlComponent::Null:
        fail(QStringLiteral("QML component did not initialize"));
        break;
    case QQmlComponent::Loading:
        break;
    }
}

void QmlObjectLoadHandle::onIncubatorStatusChanged()
{
    if (m_status != Status::Loading || !m_incubator)
        return;

    switch (m_incubator->status()) {
    case QQmlIncubator::Ready: {
        QObject *created = m_incubator->object();
        if (!created) {
            fail(QStringLiteral("QML incubation completed without an object"));
            return;
        }
        finishWithObject(created);
        break;
    }
    case QQmlIncubator::Error: {
        QStringList errors;
        for (const QQmlError &error : m_incubator->errors())
            errors.push_back(error.toString());
        fail(errors.isEmpty()
                 ? QStringLiteral("Asynchronous QML object creation failed")
                 : errors.join(QLatin1Char('\n')));
        break;
    }
    case QQmlIncubator::Null:
    case QQmlIncubator::Loading:
        break;
    }
}

void QmlObjectLoadHandle::cancel()
{
    if (m_status != Status::Loading)
        return;

    if (m_incubator)
        m_incubator->clear();
    if (m_component)
        disconnect(m_component, nullptr, this, nullptr);
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};
    m_component = nullptr;
    m_ownedComponent.reset();
    setStatus(Status::Canceled);
    emit finished();
}

void QmlObjectLoadHandle::forceCompletion()
{
    if (m_status != Status::Loading || !m_engine)
        return;

    if (m_incubator) {
        m_incubator->forceCompletion();
        onIncubatorStatusChanged();
        return;
    }

    if (m_component && m_component->status() == QQmlComponent::Ready) {
        startIncubation();
        if (m_incubator) {
            m_incubator->forceCompletion();
            onIncubatorStatusChanged();
        }
        return;
    }

    // URL-backed components can be recreated synchronously when foreground
    // access requires completion. Externally supplied components keep their
    // own loading contract; there is no safe public API to force their compile.
    if (m_source.isEmpty()) {
        if (m_component && m_component->status() == QQmlComponent::Error)
            fail(m_component->errorString());
        else
            fail(QStringLiteral("External QML component is still loading"));
        return;
    }

    if (m_component)
        disconnect(m_component, nullptr, this, nullptr);
    QObject::disconnect(m_componentDestroyedConnection);
    m_componentDestroyedConnection = {};
    m_component = nullptr;
    m_ownedComponent.reset();

    QString error;
    QObject *created = QmlObjectFactory::createSync(
        m_engine, m_source, m_initialProperties, &error, m_creationContext);
    if (!created) {
        fail(error);
        return;
    }
    finishWithObject(created);
}

QObject *QmlObjectLoadHandle::takeObject()
{
    QObject *object = m_object;
    if (m_object) {
        m_object = nullptr;
        emit objectChanged();
    }
    return object;
}

void QmlObjectLoadHandle::finishWithObject(QObject *object)
{
    if (m_status != Status::Loading || !object)
        return;

    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    m_object = object;
    emit objectChanged();
    setStatus(Status::Ready);
    emit finished();
}

void QmlObjectLoadHandle::fail(const QString &message)
{
    setErrorString(message.isEmpty()
                       ? QStringLiteral("QML object creation failed")
                       : message);
    setStatus(Status::Error);
    emit finished();
}

void QmlObjectLoadHandle::setStatus(Status status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

void QmlObjectLoadHandle::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;
    m_errorString = message;
    emit errorStringChanged();
}

QmlObjectFactory::QmlObjectFactory(QObject *parent)
    : QObject(parent)
{
}

void QmlObjectFactory::setEngine(QQmlEngine *engine)
{
    if (m_engine == engine)
        return;
    m_engine = engine;
    emit engineChanged();
}

void QmlObjectFactory::setCreationContext(QQmlContext *context)
{
    m_creationContext = context;
}

QObject *QmlObjectFactory::create(const QUrl &source,
                                  const QVariantMap &initialProperties)
{
    QString error;
    QObject *object = createSync(effectiveEngine(), source, initialProperties, &error,
                                 effectiveCreationContext());
    setLastError(error);
    return object;
}

QObject *QmlObjectFactory::createComponent(QQmlComponent *component,
                                           const QVariantMap &initialProperties)
{
    QString error;
    QObject *object = createSync(component, initialProperties, &error,
                                 effectiveCreationContext(component));
    setLastError(error);
    return object;
}

QmlObjectLoadHandle *QmlObjectFactory::createAsync(
    const QUrl &source,
    const QVariantMap &initialProperties)
{
    return beginAsync(new QmlObjectLoadHandle(source, initialProperties,
                                               effectiveCreationContext(), this),
                      effectiveEngine());
}

QmlObjectLoadHandle *QmlObjectFactory::createAsyncComponent(
    QQmlComponent *component,
    const QVariantMap &initialProperties)
{
    QQmlEngine *engine = component ? component->engine() : effectiveEngine();
    return beginAsync(new QmlObjectLoadHandle(component, initialProperties,
                                               effectiveCreationContext(component), this),
                      engine);
}

QmlObjectLoadHandle *QmlObjectFactory::beginAsync(QmlObjectLoadHandle *handle,
                                                   QQmlEngine *engine)
{
    setLastError({});
    if (engine)
        QmlIncubationScheduler::ensureForEngine(engine);

    handle->begin(engine);
    if (handle->status() == QmlObjectLoadHandle::Status::Error)
        setLastError(handle->errorString());
    else {
        connect(handle, &QmlObjectLoadHandle::errorStringChanged, this, [this, handle] {
            if (!handle->errorString().isEmpty())
                setLastError(handle->errorString());
        });
    }
    return handle;
}

QObject *QmlObjectFactory::createSync(QQmlEngine *engine,
                                      const QUrl &source,
                                      const QVariantMap &initialProperties,
                                      QString *error,
                                      QQmlContext *creationContext)
{
    auto setError = [error](const QString &message) {
        if (error)
            *error = message;
    };

    if (!engine) {
        setError(QStringLiteral("QML object creation requires a QQmlEngine"));
        return nullptr;
    }
    if (!source.isValid() || source.isEmpty()) {
        setError(QStringLiteral("QML source URL is empty or invalid"));
        return nullptr;
    }
    if (!contextMatchesEngine(creationContext, engine)) {
        setError(QStringLiteral("QML creation context belongs to a different QQmlEngine"));
        return nullptr;
    }

    QQmlComponent component(engine, source, QQmlComponent::PreferSynchronous);
    return createSync(&component, initialProperties, error, creationContext);
}

QObject *QmlObjectFactory::createSync(QQmlComponent *component,
                                      const QVariantMap &initialProperties,
                                      QString *error,
                                      QQmlContext *creationContext)
{
    auto setError = [error](const QString &message) {
        if (error)
            *error = message;
    };

    if (!component) {
        setError(QStringLiteral("QML component is null"));
        return nullptr;
    }
    if (component->status() == QQmlComponent::Loading) {
        setError(QStringLiteral("Synchronous QML creation cannot use a still-loading component"));
        return nullptr;
    }
    if (component->status() == QQmlComponent::Error) {
        setError(component->errorString());
        return nullptr;
    }
    if (component->status() != QQmlComponent::Ready) {
        setError(QStringLiteral("QML component is not ready"));
        return nullptr;
    }

    QQmlContext *context = creationContext
        ? creationContext
        : component->creationContext();
    if (!contextMatchesEngine(context, component->engine())) {
        setError(QStringLiteral("QML creation context belongs to a different QQmlEngine"));
        return nullptr;
    }

    QObject *object = component->createWithInitialProperties(initialProperties, context);
    if (!object) {
        setError(component->errorString().isEmpty()
                     ? QStringLiteral("QML component creation returned null")
                     : component->errorString());
        return nullptr;
    }

    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    setError({});
    return object;
}

QQmlEngine *QmlObjectFactory::effectiveEngine() const
{
    return m_engine ? m_engine.data() : qmlEngine(const_cast<QmlObjectFactory *>(this));
}

QQmlContext *QmlObjectFactory::effectiveCreationContext(QQmlComponent *component) const
{
    if (m_creationContext)
        return m_creationContext;
    if (component && component->creationContext())
        return component->creationContext();
    return QQmlEngine::contextForObject(const_cast<QmlObjectFactory *>(this));
}

void QmlObjectFactory::setLastError(const QString &message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}

} // namespace LfQuick
