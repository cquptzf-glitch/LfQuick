#include <LfQuick/Ui/QmlObjectFactory.h>

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlIncubator>
#include <QtQml/qqml.h>

#include <utility>

namespace LfQuick {

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
                                         QObject *parent)
    : QObject(parent)
    , m_source(std::move(source))
    , m_initialProperties(std::move(initialProperties))
{
}

QmlObjectLoadHandle::~QmlObjectLoadHandle()
{
    if (m_status == Status::Loading && m_incubator)
        m_incubator->clear();
    if (m_object)
        m_object->deleteLater();
}

void QmlObjectLoadHandle::begin(QQmlEngine *engine)
{
    m_engine = engine;
    if (!m_engine) {
        fail(QStringLiteral("QML object creation requires a QQmlEngine"));
        return;
    }
    if (!m_source.isValid() || m_source.isEmpty()) {
        fail(QStringLiteral("QML source URL is empty or invalid"));
        return;
    }

    m_component = std::make_unique<QQmlComponent>(
        m_engine, m_source, QQmlComponent::Asynchronous);
    connect(m_component.get(), &QQmlComponent::statusChanged,
            this, &QmlObjectLoadHandle::onComponentStatusChanged);
    onComponentStatusChanged();
}

void QmlObjectLoadHandle::startIncubation()
{
    if (m_status != Status::Loading || !m_component || !m_engine || m_incubator)
        return;

    m_incubator = std::make_unique<FactoryIncubator>(this);
    // Use QQmlIncubator's dedicated initial-property path instead of applying
    // properties manually from setInitialState(). This keeps asynchronous
    // creation aligned with QQmlComponent::createWithInitialProperties(),
    // including required-property validation and pre-completion semantics.
    m_incubator->setInitialProperties(m_initialProperties);
    m_component->create(*m_incubator, m_engine->rootContext());
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
    m_component.reset();
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

    // A component may still be compiling asynchronously. Foreground access is
    // explicitly allowed to block, so abandon that compilation and recreate it
    // synchronously. This mirrors LazyLoader's "item access forces completion"
    // behavior while keeping the common background path asynchronous.
    if (m_component)
        disconnect(m_component.get(), nullptr, this, nullptr);
    m_component.reset();

    QString error;
    QObject *created = QmlObjectFactory::createSync(
        m_engine, m_source, m_initialProperties, &error);
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

QObject *QmlObjectFactory::create(const QUrl &source,
                                  const QVariantMap &initialProperties)
{
    QString error;
    QObject *object = createSync(effectiveEngine(), source, initialProperties, &error);
    setLastError(error);
    return object;
}

QmlObjectLoadHandle *QmlObjectFactory::createAsync(
    const QUrl &source,
    const QVariantMap &initialProperties)
{
    setLastError({});
    auto *handle = new QmlObjectLoadHandle(source, initialProperties, this);
    handle->begin(effectiveEngine());
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
                                      QString *error)
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

    QQmlComponent component(engine, source, QQmlComponent::PreferSynchronous);
    if (component.status() == QQmlComponent::Loading) {
        setError(QStringLiteral("Synchronous QML creation cannot use a still-loading source"));
        return nullptr;
    }
    if (component.status() == QQmlComponent::Error) {
        setError(component.errorString());
        return nullptr;
    }

    QObject *object = component.createWithInitialProperties(initialProperties);
    if (!object) {
        setError(component.errorString().isEmpty()
                     ? QStringLiteral("QML component creation returned null")
                     : component.errorString());
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

void QmlObjectFactory::setLastError(const QString &message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}

} // namespace LfQuick
