#include <LfQuick/Ui/QuickWindowProvider.h>

#include <LfQuick/Runtime/IWindowBacking.h>
#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QmlObjectFactory.h>
#include <LfQuick/Ui/QmlStateSnapshot.h>

#include <QCloseEvent>
#include <QColor>
#include <QList>
#include <QMetaObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRect>
#include <QUrl>
#include <QUuid>

#include <algorithm>
#include <utility>

namespace LfQuick {
namespace {

QUrl sourceUrl(const WindowDescriptor &descriptor)
{
    const QVariant raw = descriptor.source.value(QStringLiteral("url"));
    if (raw.canConvert<QUrl>())
        return raw.toUrl();
    return QUrl(raw.toString());
}

QQuickItem *createContent(QQmlEngine *engine,
                          const QUrl &source,
                          const QVariantMap &properties,
                          QString &error)
{
    QObject *object = QmlObjectFactory::createSync(engine, source, properties, &error);
    if (!object)
        return nullptr;

    auto *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        error = QStringLiteral("Quick window content root must inherit QQuickItem");
        object->deleteLater();
        return nullptr;
    }

    item->setVisible(false);
    return item;
}

void invokeOptional(QObject *object, const char *method)
{
    if (!object || !method)
        return;
    const QByteArray signature = QByteArray(method) + QByteArrayLiteral("()");
    if (object->metaObject()->indexOfMethod(signature.constData()) < 0)
        return;
    QMetaObject::invokeMethod(object, method, Qt::DirectConnection);
}

int configuredInt(const WindowDescriptor &descriptor,
                  const QString &key,
                  int fallback)
{
    bool ok = false;
    const int value = descriptor.source.value(key).toInt(&ok);
    return ok ? value : fallback;
}

} // namespace

struct QuickWindowProviderState
{
    QHash<WindowInstanceId, QPointer<QQuickWindow>> windows;
};

namespace {

class QuickWindowBacking final : public IWindowBacking
{
public:
    QuickWindowBacking(WindowInstanceId id,
                       QQuickWindow *window,
                       std::shared_ptr<QuickWindowProviderState> state)
        : m_id(std::move(id))
        , m_window(window)
        , m_state(std::move(state))
    {
        if (m_window) {
            m_destroyedConnection = QObject::connect(
                m_window, &QObject::destroyed, [this] {
                    if (m_state)
                        m_state->windows.remove(m_id);
                    m_window = nullptr;
                });
        }
    }

    ~QuickWindowBacking() override
    {
        QObject::disconnect(m_destroyedConnection);
        if (!m_window)
            return;

        QQuickWindow *window = m_window.data();
        if (m_state && m_state->windows.value(m_id) == window)
            m_state->windows.remove(m_id);
        m_window = nullptr;
        window->deleteLater();
    }

    QString providerId() const override
    {
        return QStringLiteral("quick-window");
    }

    WindowInstanceId instanceId() const override { return m_id; }

    bool valid() const noexcept { return !m_window.isNull(); }
    QQuickWindow *window() const noexcept { return m_window.data(); }
    const std::shared_ptr<QuickWindowProviderState> &state() const noexcept
    {
        return m_state;
    }

    QQuickWindow *takeWindow()
    {
        QObject::disconnect(m_destroyedConnection);
        m_destroyedConnection = {};
        QQuickWindow *window = m_window.data();
        m_window = nullptr;
        return window;
    }

private:
    WindowInstanceId m_id;
    QPointer<QQuickWindow> m_window;
    std::shared_ptr<QuickWindowProviderState> m_state;
    QMetaObject::Connection m_destroyedConnection;
};

class QuickWindowInstance final : public QObject, public IWindowInstance
{
public:
    QuickWindowInstance(WindowInstanceId id,
                        QQuickWindow *window,
                        QQuickItem *content,
                        std::shared_ptr<QuickWindowProviderState> state)
        : m_id(std::move(id))
        , m_state(std::move(state))
    {
        attachWindow(window);
        attachContent(content);
    }

    ~QuickWindowInstance() override
    {
        QQuickWindow *window = m_window.data();
        detachWindowHooks();
        destroyContent();
        if (window && m_state && m_state->windows.value(m_id) == window)
            m_state->windows.remove(m_id);
        m_window = nullptr;
        if (window)
            window->deleteLater();
    }

    WindowInstanceId instanceId() const override { return m_id; }

    void activate() override
    {
        if (!m_window)
            return;
        if (m_content)
            m_content->setVisible(true);
        m_window->show();
        m_window->requestActivate();
        invokeOptional(m_content, "onWindowActivated");
    }

    void deactivate() override
    {
        invokeOptional(m_content, "onWindowDeactivated");
    }

    bool canClose(QString *reason) const override
    {
        if (!m_content)
            return true;

        const QByteArray signature("canCloseWindow()");
        if (m_content->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            bool allowed = true;
            QMetaObject::invokeMethod(m_content, "canCloseWindow",
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(bool, allowed));
            if (!allowed && reason)
                *reason = m_content->property("lfquickWindowCloseReason").toString();
            return allowed;
        }

        if (m_content->property("lfquickWindowCloseBlocked").toBool()) {
            if (reason)
                *reason = m_content->property("lfquickWindowCloseReason").toString();
            return false;
        }
        return true;
    }

    void setCloseRequestHandler(CloseRequestHandler handler) override
    {
        m_closeRequestHandler = std::move(handler);
    }

    bool canUpdateOwner(const WindowInstanceId &ownerInstanceId) const override
    {
        if (!m_window || !m_state)
            return false;
        if (ownerInstanceId.isEmpty())
            return true;
        QQuickWindow *owner = m_state->windows.value(ownerInstanceId);
        return owner && owner != m_window;
    }

    bool updateOwner(const WindowInstanceId &ownerInstanceId) override
    {
        if (!canUpdateOwner(ownerInstanceId))
            return false;

        QWindow *owner = ownerInstanceId.isEmpty()
            ? nullptr
            : m_state->windows.value(ownerInstanceId);
        m_window->setTransientParent(owner);
        m_window->setProperty("lfquickWindowOwnerInstanceId", ownerInstanceId);
        return true;
    }

    bool updateParameters(const QVariantMap &parameters) override
    {
        if (!m_content)
            return parameters.isEmpty();

        bool accepted = true;
        for (auto it = parameters.cbegin(); it != parameters.cend(); ++it) {
            const QByteArray name = it.key().toUtf8();
            if (!m_content->setProperty(name.constData(), it.value()))
                accepted = false;
        }
        m_content->setProperty("lfquickWindowParameters", parameters);

        const QByteArray signature("onWindowParametersChanged(QVariantMap)");
        if (m_content->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            QMetaObject::invokeMethod(m_content, "onWindowParametersChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QVariantMap, parameters));
        }
        return accepted;
    }

    QVariantMap saveState() const override
    {
        if (!m_window)
            return {};

        QVariantMap state{
            {QStringLiteral("geometry"), m_window->geometry()},
            {QStringLiteral("windowState"), int(m_window->windowState())},
            {QStringLiteral("contentVisible"), m_content && m_content->isVisible()}
        };
        if (m_content)
            state.insert(QStringLiteral("content"), QmlStateSnapshot::capture(m_content));
        return state;
    }

    bool restoreState(const QVariantMap &state) override
    {
        if (!m_window)
            return false;

        if (state.contains(QStringLiteral("geometry"))) {
            const QRect geometry = state.value(QStringLiteral("geometry")).toRect();
            if (geometry.isValid())
                m_window->setGeometry(geometry);
        }
        if (state.contains(QStringLiteral("windowState"))) {
            m_window->setWindowState(
                static_cast<Qt::WindowState>(state.value(QStringLiteral("windowState")).toInt()));
        }
        if (m_content && state.contains(QStringLiteral("content")))
            QmlStateSnapshot::restore(m_content, state.value(QStringLiteral("content")).toMap());
        if (m_content && state.contains(QStringLiteral("contentVisible")))
            m_content->setVisible(state.value(QStringLiteral("contentVisible")).toBool());
        return true;
    }

    bool canReleaseBacking() const noexcept override
    {
        return !m_window.isNull();
    }

    std::unique_ptr<IWindowBacking> releaseBacking() override
    {
        if (!m_window)
            return {};

        QQuickWindow *window = m_window.data();
        detachContentHooks();
        detachWindowHooks();
        m_window = nullptr;
        return std::make_unique<QuickWindowBacking>(m_id, window, m_state);
    }

    bool canAdoptBacking(const IWindowBacking &backing) const noexcept override
    {
        const auto *quickBacking = dynamic_cast<const QuickWindowBacking *>(&backing);
        return !m_window
            && quickBacking
            && quickBacking->valid()
            && quickBacking->instanceId() == m_id
            && quickBacking->state() == m_state;
    }

    bool adoptBacking(std::unique_ptr<IWindowBacking> &backing) override
    {
        if (!backing || !canAdoptBacking(*backing))
            return false;

        auto *quickBacking = dynamic_cast<QuickWindowBacking *>(backing.get());
        QQuickWindow *window = quickBacking ? quickBacking->takeWindow() : nullptr;
        if (!window)
            return false;

        backing.reset();
        attachWindow(window);
        if (m_content)
            attachContent(m_content);
        return true;
    }

    void close() override
    {
        if (!m_window) {
            destroyContent();
            return;
        }

        QQuickWindow *window = m_window.data();
        invokeOptional(m_content, "onWindowClosing");
        m_closingFromRouter = true;
        window->close();

        if (m_window) {
            detachWindowHooks();
            m_window = nullptr;
            destroyContent();
            if (m_state && m_state->windows.value(m_id) == window)
                m_state->windows.remove(m_id);
            window->deleteLater();
        } else {
            // Native/custom close handlers may synchronously destroy the window
            // and its content hierarchy.
            m_content = nullptr;
            m_contentConnections.clear();
        }
        m_closingFromRouter = false;
    }

private:
    void attachWindow(QQuickWindow *window)
    {
        m_window = window;
        if (!m_window)
            return;

        m_closingConnection = connect(
            m_window, &QWindow::closing, this, [this](QCloseEvent *event) {
                if (m_closingFromRouter || !m_closeRequestHandler)
                    return;
                if (event)
                    event->ignore();
                const auto handler = m_closeRequestHandler;
                QMetaObject::invokeMethod(this, [handler] {
                    if (handler)
                        handler();
                }, Qt::QueuedConnection);
            });

        m_destroyedConnection = connect(m_window, &QObject::destroyed, this, [this] {
            m_window = nullptr;
            m_content = nullptr;
            m_contentConnections.clear();
            m_closingConnection = {};
            m_destroyedConnection = {};
            if (m_state)
                m_state->windows.remove(m_id);

            if (!m_closingFromRouter && m_closeRequestHandler) {
                const auto handler = m_closeRequestHandler;
                QMetaObject::invokeMethod(this, [handler] {
                    if (handler)
                        handler();
                }, Qt::QueuedConnection);
            }
        });
    }

    void attachContent(QQuickItem *content)
    {
        detachContentHooks();
        if (content)
            m_content = content;
        if (!m_content || !m_window)
            return;

        QQuickItem *root = m_window->contentItem();
        if (m_content->parentItem() != root)
            m_content->setParentItem(root);
        if (m_content->parent() != root)
            m_content->setParent(root);

        m_content->setSize(QSizeF(m_window->size()));
        const QPointer<QQuickItem> guardedContent = m_content;
        const QPointer<QQuickWindow> guardedWindow = m_window;
        m_contentConnections.push_back(connect(
            m_window, &QQuickWindow::widthChanged, m_content,
            [guardedContent, guardedWindow] {
                if (guardedContent && guardedWindow)
                    guardedContent->setWidth(guardedWindow->width());
            }));
        m_contentConnections.push_back(connect(
            m_window, &QQuickWindow::heightChanged, m_content,
            [guardedContent, guardedWindow] {
                if (guardedContent && guardedWindow)
                    guardedContent->setHeight(guardedWindow->height());
            }));
    }

    void detachContentHooks()
    {
        for (const QMetaObject::Connection &connection : std::as_const(m_contentConnections))
            QObject::disconnect(connection);
        m_contentConnections.clear();
    }

    void detachWindowHooks()
    {
        QObject::disconnect(m_closingConnection);
        QObject::disconnect(m_destroyedConnection);
        m_closingConnection = {};
        m_destroyedConnection = {};
    }

    void destroyContent()
    {
        detachContentHooks();
        if (!m_content)
            return;
        QQuickItem *content = m_content.data();
        m_content = nullptr;
        content->setVisible(false);
        content->setParentItem(nullptr);
        content->setParent(nullptr);
        content->deleteLater();
    }

    WindowInstanceId m_id;
    QPointer<QQuickWindow> m_window;
    QPointer<QQuickItem> m_content;
    std::shared_ptr<QuickWindowProviderState> m_state;
    CloseRequestHandler m_closeRequestHandler;
    QMetaObject::Connection m_closingConnection;
    QMetaObject::Connection m_destroyedConnection;
    QList<QMetaObject::Connection> m_contentConnections;
    bool m_closingFromRouter = false;
};

} // namespace

QuickWindowProvider::QuickWindowProvider(QQmlEngine *engine)
    : m_engine(engine)
    , m_state(std::make_shared<QuickWindowProviderState>())
{
}

QuickWindowProvider::~QuickWindowProvider() = default;

QString QuickWindowProvider::id() const
{
    return QStringLiteral("quick-window");
}

bool QuickWindowProvider::canCreate(const WindowDescriptor &descriptor) const
{
    const QUrl url = resolvedSource(descriptor);
    return m_engine
        && descriptor.provider == id()
        && url.isValid()
        && !url.isEmpty();
}

std::unique_ptr<IWindowInstance> QuickWindowProvider::create(
    const WindowDescriptor &descriptor,
    const WindowOpenRequest &request)
{
    clearLastError();
    const QUrl url = resolvedSource(descriptor);
    if (!canCreate(descriptor)) {
        setLastError(QStringLiteral("quick-window.invalid-descriptor"),
                     QStringLiteral("Invalid Quick window descriptor or QQmlEngine"),
                     {{QStringLiteral("windowId"), descriptor.id},
                      {QStringLiteral("sourceUrl"), url.toString()}});
        return {};
    }

    QString error;
    QQuickItem *content = createContent(m_engine, url, request.parameters, error);
    if (!content) {
        setLastError(QStringLiteral("quick-window.content-create-failed"), error,
                     {{QStringLiteral("windowId"), descriptor.id},
                      {QStringLiteral("sourceUrl"), url.toString()}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.id, url, error, QStringLiteral("window-content"));
        return {};
    }

    auto *window = new QQuickWindow;
    window->setTitle(descriptor.source.value(
        QStringLiteral("title"), descriptor.id).toString());

    const int implicitWidth = std::max(1, qRound(content->implicitWidth()));
    const int implicitHeight = std::max(1, qRound(content->implicitHeight()));
    window->resize(configuredInt(descriptor, QStringLiteral("width"),
                                 implicitWidth > 1 ? implicitWidth : 800),
                   configuredInt(descriptor, QStringLiteral("height"),
                                 implicitHeight > 1 ? implicitHeight : 600));

    if (descriptor.source.contains(QStringLiteral("x")))
        window->setX(configuredInt(descriptor, QStringLiteral("x"), window->x()));
    if (descriptor.source.contains(QStringLiteral("y")))
        window->setY(configuredInt(descriptor, QStringLiteral("y"), window->y()));
    if (descriptor.source.contains(QStringLiteral("color"))) {
        const QColor color = descriptor.source.value(QStringLiteral("color")).value<QColor>();
        if (color.isValid())
            window->setColor(color);
    }

    QQuickWindow *owner = nullptr;
    if (!request.ownerInstanceId.isEmpty()) {
        owner = m_state->windows.value(request.ownerInstanceId);
        if (!owner) {
            content->deleteLater();
            window->deleteLater();
            setLastError(QStringLiteral("quick-window.owner-incompatible"),
                         QStringLiteral("Quick window owner is not managed by this provider"),
                         {{QStringLiteral("windowId"), descriptor.id},
                          {QStringLiteral("ownerInstanceId"), request.ownerInstanceId}});
            return {};
        }
        window->setTransientParent(owner);
    }

    const WindowInstanceId instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    window->setProperty("lfquickWindowInstanceId", instanceId);
    window->setProperty("lfquickWindowId", descriptor.id);
    window->setProperty("lfquickWindowOwnerInstanceId", request.ownerInstanceId);
    m_state->windows.insert(instanceId, window);

    return std::make_unique<QuickWindowInstance>(
        instanceId, window, content, m_state);
}

bool QuickWindowProvider::canAdoptBacking(const WindowDescriptor &descriptor,
                                          const IWindowBacking &backing) const
{
    const auto *quickBacking = dynamic_cast<const QuickWindowBacking *>(&backing);
    return descriptor.provider == id()
        && quickBacking
        && quickBacking->valid()
        && quickBacking->providerId() == id()
        && quickBacking->state() == m_state;
}

std::unique_ptr<IWindowInstance> QuickWindowProvider::createWithBacking(
    const WindowDescriptor &descriptor,
    const WindowOpenRequest &request,
    std::unique_ptr<IWindowBacking> &backing)
{
    clearLastError();
    if (!backing || !canAdoptBacking(descriptor, *backing)) {
        setLastError(QStringLiteral("quick-window.backing-rejected"),
                     QStringLiteral("Quick window backing is incompatible"),
                     {{QStringLiteral("windowId"), descriptor.id}});
        return {};
    }

    const QUrl url = resolvedSource(descriptor);
    QString error;
    QQuickItem *content = createContent(m_engine, url, request.parameters, error);
    if (!content) {
        setLastError(QStringLiteral("quick-window.content-create-failed"), error,
                     {{QStringLiteral("windowId"), descriptor.id},
                      {QStringLiteral("sourceUrl"), url.toString()},
                      {QStringLiteral("stage"), QStringLiteral("backing-reload")}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.id, url, error,
                                     QStringLiteral("window-backing-reload"));
        return {};
    }

    auto *quickBacking = dynamic_cast<QuickWindowBacking *>(backing.get());
    const WindowInstanceId instanceId = quickBacking->instanceId();
    QQuickWindow *window = quickBacking->takeWindow();
    if (!window) {
        content->deleteLater();
        setLastError(QStringLiteral("quick-window.backing-empty"),
                     QStringLiteral("Quick window backing no longer contains a QQuickWindow"));
        return {};
    }

    backing.reset();
    m_state->windows.insert(instanceId, window);
    return std::make_unique<QuickWindowInstance>(
        instanceId, window, content, m_state);
}

void QuickWindowProvider::setErrorCollector(QmlErrorCollector *collector)
{
    m_errorCollector = collector;
}

void QuickWindowProvider::setSourceResolver(SourceResolver resolver)
{
    m_sourceResolver = std::move(resolver);
}

QUrl QuickWindowProvider::resolvedSource(const WindowDescriptor &descriptor) const
{
    const QUrl logical = sourceUrl(descriptor);
    if (!m_sourceResolver || logical.isEmpty())
        return logical;

    const QUrl resolved = m_sourceResolver(logical);
    return resolved.isValid() && !resolved.isEmpty() ? resolved : logical;
}

void QuickWindowProvider::clearLastError()
{
    m_lastErrorInfo = {};
}

void QuickWindowProvider::setLastError(QString code,
                                       QString message,
                                       QVariantMap details)
{
    m_lastErrorInfo.code = std::move(code);
    m_lastErrorInfo.message = std::move(message);
    m_lastErrorInfo.details = std::move(details);
}

} // namespace LfQuick
