#include <LfQuick/WidgetBridge/WidgetWindowProvider.h>

#include <LfQuick/Runtime/IWindowBacking.h>
#include <LfQuick/Runtime/IWindowInstance.h>

#include <QByteArray>
#include <QEvent>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QUuid>
#include <QWidget>
#include <QWindow>

#include <utility>

namespace LfQuick {
namespace {

QString factoryId(const WindowDescriptor &descriptor)
{
    return descriptor.source.value(QStringLiteral("factory")).toString();
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

} // namespace

struct WidgetWindowProviderState
{
    QHash<WindowInstanceId, QPointer<QWidget>> widgets;
};

namespace {

class WidgetWindowBacking final : public IWindowBacking
{
public:
    WidgetWindowBacking(WindowInstanceId id,
                        QWidget *widget,
                        std::shared_ptr<WidgetWindowProviderState> state)
        : m_id(std::move(id))
        , m_widget(widget)
        , m_state(std::move(state))
    {
        if (m_widget) {
            m_destroyedConnection = QObject::connect(
                m_widget, &QObject::destroyed, [this] {
                    if (m_state)
                        m_state->widgets.remove(m_id);
                    m_widget = nullptr;
                });
        }
    }

    ~WidgetWindowBacking() override
    {
        QObject::disconnect(m_destroyedConnection);
        if (!m_widget)
            return;

        QWidget *widget = m_widget.data();
        if (m_state && m_state->widgets.value(m_id) == widget)
            m_state->widgets.remove(m_id);
        m_widget = nullptr;
        widget->deleteLater();
    }

    QString providerId() const override
    {
        return QStringLiteral("widget-window");
    }

    WindowInstanceId instanceId() const override { return m_id; }

    bool valid() const noexcept { return !m_widget.isNull(); }
    QWidget *widget() const noexcept { return m_widget.data(); }
    const std::shared_ptr<WidgetWindowProviderState> &state() const noexcept
    {
        return m_state;
    }

    QWidget *takeWidget()
    {
        QObject::disconnect(m_destroyedConnection);
        m_destroyedConnection = {};
        QWidget *widget = m_widget.data();
        m_widget = nullptr;
        return widget;
    }

private:
    WindowInstanceId m_id;
    QPointer<QWidget> m_widget;
    std::shared_ptr<WidgetWindowProviderState> m_state;
    QMetaObject::Connection m_destroyedConnection;
};

class WidgetWindowInstance final : public QObject, public IWindowInstance
{
public:
    WidgetWindowInstance(WindowInstanceId id,
                         QWidget *widget,
                         std::shared_ptr<WidgetWindowProviderState> state)
        : m_id(std::move(id))
        , m_state(std::move(state))
    {
        attachWidget(widget);
    }

    ~WidgetWindowInstance() override
    {
        QWidget *widget = m_widget.data();
        detachWidgetHooks();
        if (widget && m_state && m_state->widgets.value(m_id) == widget)
            m_state->widgets.remove(m_id);
        m_widget = nullptr;
        if (widget)
            widget->deleteLater();
    }

    WindowInstanceId instanceId() const override { return m_id; }

    void activate() override
    {
        if (!m_widget)
            return;
        m_widget->show();
        if (m_widget->isMinimized())
            m_widget->showNormal();
        m_widget->raise();
        m_widget->activateWindow();
        invokeOptional(m_widget, "onWindowActivated");
    }

    void deactivate() override
    {
        invokeOptional(m_widget, "onWindowDeactivated");
    }

    bool canClose(QString *reason) const override
    {
        if (!m_widget)
            return true;

        const QByteArray signature("canCloseWindow()");
        if (m_widget->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            bool allowed = true;
            QMetaObject::invokeMethod(m_widget, "canCloseWindow",
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(bool, allowed));
            if (!allowed && reason)
                *reason = m_widget->property("lfquickWindowCloseReason").toString();
            return allowed;
        }

        if (m_widget->property("lfquickWindowCloseBlocked").toBool()) {
            if (reason)
                *reason = m_widget->property("lfquickWindowCloseReason").toString();
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
        if (!m_widget || !m_state)
            return false;
        if (ownerInstanceId.isEmpty())
            return true;
        const QWidget *owner = m_state->widgets.value(ownerInstanceId);
        return owner && owner != m_widget;
    }

    bool updateOwner(const WindowInstanceId &ownerInstanceId) override
    {
        if (!canUpdateOwner(ownerInstanceId))
            return false;

        QWidget *owner = ownerInstanceId.isEmpty()
            ? nullptr
            : m_state->widgets.value(ownerInstanceId);

        const bool wasVisible = m_widget->isVisible();
        const QRect previousGeometry = m_widget->geometry();
        if (m_widget->parentWidget())
            m_widget->setParent(nullptr, m_widget->windowFlags() | Qt::Window);

        // QWidget parentage controls QObject lifetime, which is too strong for
        // Runtime OwnerClosePolicy::Detach. Use the native transient relation
        // instead so Runtime remains authoritative over lifetime.
        m_widget->winId();
        if (owner)
            owner->winId();
        QWindow *windowHandle = m_widget->windowHandle();
        QWindow *ownerHandle = owner ? owner->windowHandle() : nullptr;
        if (!windowHandle)
            return false;
        if (owner && !ownerHandle)
            return false;
        windowHandle->setTransientParent(ownerHandle);

        m_widget->setProperty("lfquickWindowOwnerInstanceId", ownerInstanceId);
        m_widget->setGeometry(previousGeometry);
        if (wasVisible)
            m_widget->show();
        return true;
    }

    bool updateParameters(const QVariantMap &parameters) override
    {
        if (!m_widget)
            return false;
        m_widget->setProperty("lfquickWindowParameters", parameters);
        const QByteArray signature("onWindowParametersChanged(QVariantMap)");
        if (m_widget->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            QMetaObject::invokeMethod(m_widget, "onWindowParametersChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QVariantMap, parameters));
        }
        return true;
    }

    bool restoreState(const QVariantMap &state) override
    {
        if (!m_widget)
            return false;

        bool restored = true;
        const QByteArray geometry = state.value(QStringLiteral("geometry")).toByteArray();
        if (!geometry.isEmpty())
            restored = m_widget->restoreGeometry(geometry) && restored;

        if (state.contains(QStringLiteral("windowState")))
            m_widget->setWindowState(Qt::WindowStates(state.value(QStringLiteral("windowState")).toInt()));

        m_widget->setProperty("lfquickWindowRestoreState", state);
        const QByteArray signature("onWindowStateRestored(QVariantMap)");
        if (m_widget->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            QMetaObject::invokeMethod(m_widget, "onWindowStateRestored",
                                      Qt::DirectConnection,
                                      Q_ARG(QVariantMap, state));
        }
        return restored;
    }

    QVariantMap saveState() const override
    {
        if (!m_widget)
            return {};
        return {
            {QStringLiteral("geometry"), m_widget->saveGeometry()},
            {QStringLiteral("windowState"), int(m_widget->windowState())}
        };
    }

    bool canReleaseBacking() const noexcept override
    {
        return !m_widget.isNull();
    }

    std::unique_ptr<IWindowBacking> releaseBacking() override
    {
        if (!m_widget)
            return {};

        QWidget *widget = m_widget.data();
        detachWidgetHooks();
        m_widget = nullptr;
        return std::make_unique<WidgetWindowBacking>(m_id, widget, m_state);
    }

    bool canAdoptBacking(const IWindowBacking &backing) const noexcept override
    {
        const auto *widgetBacking = dynamic_cast<const WidgetWindowBacking *>(&backing);
        return !m_widget
            && widgetBacking
            && widgetBacking->valid()
            && widgetBacking->instanceId() == m_id
            && widgetBacking->state() == m_state;
    }

    bool adoptBacking(std::unique_ptr<IWindowBacking> &backing) override
    {
        if (!backing || !canAdoptBacking(*backing))
            return false;

        auto *widgetBacking = dynamic_cast<WidgetWindowBacking *>(backing.get());
        QWidget *widget = widgetBacking ? widgetBacking->takeWidget() : nullptr;
        if (!widget)
            return false;

        backing.reset();
        attachWidget(widget);
        return true;
    }

    void close() override
    {
        if (!m_widget)
            return;

        QWidget *widget = m_widget.data();
        invokeOptional(widget, "onWindowClosing");
        m_closingFromRouter = true;
        widget->close();

        // close() may synchronously destroy a custom QWidget. QPointer protects
        // the follow-up path, while WA_DeleteOnClose=false covers normal usage.
        if (m_widget) {
            detachWidgetHooks();
            m_widget = nullptr;
            if (m_state && m_state->widgets.value(m_id) == widget)
                m_state->widgets.remove(m_id);
            widget->deleteLater();
        }
        m_closingFromRouter = false;
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_widget && event && event->type() == QEvent::Close
            && !m_closingFromRouter && m_closeRequestHandler) {
            const auto handler = m_closeRequestHandler;
            event->ignore();
            QMetaObject::invokeMethod(this, [handler] {
                if (handler)
                    handler();
            }, Qt::QueuedConnection);
            return true;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void attachWidget(QWidget *widget)
    {
        m_widget = widget;
        if (!m_widget)
            return;

        m_widget->installEventFilter(this);
        m_widgetDestroyedConnection = connect(m_widget, &QObject::destroyed, this, [this] {
            m_widget = nullptr;
            m_widgetDestroyedConnection = {};
            if (m_state)
                m_state->widgets.remove(m_id);

            if (!m_closingFromRouter && m_closeRequestHandler) {
                const auto handler = m_closeRequestHandler;
                QMetaObject::invokeMethod(this, [handler] {
                    if (handler)
                        handler();
                }, Qt::QueuedConnection);
            }
        });
    }

    void detachWidgetHooks()
    {
        QObject::disconnect(m_widgetDestroyedConnection);
        m_widgetDestroyedConnection = {};
        if (m_widget)
            m_widget->removeEventFilter(this);
    }

    WindowInstanceId m_id;
    QPointer<QWidget> m_widget;
    std::shared_ptr<WidgetWindowProviderState> m_state;
    CloseRequestHandler m_closeRequestHandler;
    QMetaObject::Connection m_widgetDestroyedConnection;
    bool m_closingFromRouter = false;
};

} // namespace

WidgetWindowProvider::WidgetWindowProvider()
    : m_state(std::make_shared<WidgetWindowProviderState>())
{
}

WidgetWindowProvider::~WidgetWindowProvider() = default;

QString WidgetWindowProvider::id() const
{
    return QStringLiteral("widget-window");
}

bool WidgetWindowProvider::canCreate(const WindowDescriptor &descriptor) const
{
    return descriptor.provider == id() && hasFactory(factoryId(descriptor));
}

std::unique_ptr<IWindowInstance> WidgetWindowProvider::create(
    const WindowDescriptor &descriptor,
    const WindowOpenRequest &request)
{
    m_lastErrorInfo = {};

    const QString key = factoryId(descriptor);
    const auto it = m_factories.constFind(key);
    if (it == m_factories.cend()) {
        m_lastErrorInfo.code = QStringLiteral("widget-window.factory-not-found");
        m_lastErrorInfo.message = QStringLiteral("No widget window factory registered for '%1'").arg(key);
        m_lastErrorInfo.details = {
            {QStringLiteral("windowId"), descriptor.id},
            {QStringLiteral("factory"), key}
        };
        return {};
    }

    QWidget *owner = nullptr;
    if (!request.ownerInstanceId.isEmpty()) {
        owner = m_state->widgets.value(request.ownerInstanceId);
        if (!owner) {
            m_lastErrorInfo.code = QStringLiteral("widget-window.owner-incompatible");
            m_lastErrorInfo.message = QStringLiteral(
                "Widget window owner '%1' is not managed by this provider")
                .arg(request.ownerInstanceId);
            m_lastErrorInfo.details = {
                {QStringLiteral("windowId"), descriptor.id},
                {QStringLiteral("ownerInstanceId"), request.ownerInstanceId}
            };
            return {};
        }
    }

    QWidget *widget = it.value()(owner, request.parameters);
    if (!widget) {
        m_lastErrorInfo.code = QStringLiteral("widget-window.factory-returned-null");
        m_lastErrorInfo.message = QStringLiteral("Widget window factory '%1' returned null").arg(key);
        m_lastErrorInfo.details = {
            {QStringLiteral("windowId"), descriptor.id},
            {QStringLiteral("factory"), key}
        };
        return {};
    }

    // Runtime owns window lifetime. A QWidget QObject parent would introduce a
    // second, implicit ownership tree and break Detach semantics.
    if (widget->parentWidget())
        widget->setParent(nullptr, widget->windowFlags() | Qt::Window);
    widget->setAttribute(Qt::WA_DeleteOnClose, false);
    widget->setProperty("lfquickWindowParameters", request.parameters);

    const WindowInstanceId instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    widget->setProperty("lfquickWindowInstanceId", instanceId);
    widget->setProperty("lfquickWindowId", descriptor.id);
    m_state->widgets.insert(instanceId, widget);

    auto instance = std::make_unique<WidgetWindowInstance>(instanceId, widget, m_state);
    if (!request.ownerInstanceId.isEmpty() && !instance->updateOwner(request.ownerInstanceId)) {
        m_lastErrorInfo.code = QStringLiteral("widget-window.owner-bind-failed");
        m_lastErrorInfo.message = QStringLiteral("Widget window could not bind its native owner");
        m_lastErrorInfo.details = {
            {QStringLiteral("windowId"), descriptor.id},
            {QStringLiteral("ownerInstanceId"), request.ownerInstanceId}
        };
        return {};
    }

    return instance;
}

bool WidgetWindowProvider::canAdoptBacking(const WindowDescriptor &descriptor,
                                           const IWindowBacking &backing) const
{
    const auto *widgetBacking = dynamic_cast<const WidgetWindowBacking *>(&backing);
    return descriptor.provider == id()
        && widgetBacking
        && widgetBacking->valid()
        && widgetBacking->providerId() == id()
        && widgetBacking->state() == m_state;
}

std::unique_ptr<IWindowInstance> WidgetWindowProvider::createWithBacking(
    const WindowDescriptor &descriptor,
    const WindowOpenRequest &request,
    std::unique_ptr<IWindowBacking> &backing)
{
    m_lastErrorInfo = {};
    if (!backing || !canAdoptBacking(descriptor, *backing)) {
        m_lastErrorInfo.code = QStringLiteral("widget-window.backing-rejected");
        m_lastErrorInfo.message = QStringLiteral("Widget window backing is incompatible");
        m_lastErrorInfo.details = {{QStringLiteral("windowId"), descriptor.id}};
        return {};
    }

    auto *widgetBacking = dynamic_cast<WidgetWindowBacking *>(backing.get());
    const WindowInstanceId instanceId = widgetBacking->instanceId();
    QWidget *widget = widgetBacking->takeWidget();
    if (!widget) {
        m_lastErrorInfo.code = QStringLiteral("widget-window.backing-empty");
        m_lastErrorInfo.message = QStringLiteral("Widget window backing no longer contains a QWidget");
        return {};
    }

    backing.reset();
    widget->setProperty("lfquickWindowParameters", request.parameters);
    widget->setProperty("lfquickWindowInstanceId", instanceId);
    widget->setProperty("lfquickWindowId", descriptor.id);
    m_state->widgets.insert(instanceId, widget);
    return std::make_unique<WidgetWindowInstance>(instanceId, widget, m_state);
}

bool WidgetWindowProvider::registerFactory(QString id, Factory factory)
{
    if (id.isEmpty() || !factory || m_factories.contains(id))
        return false;
    m_factories.insert(std::move(id), std::move(factory));
    return true;
}

bool WidgetWindowProvider::unregisterFactory(QStringView id)
{
    return m_factories.remove(id.toString());
}

bool WidgetWindowProvider::hasFactory(QStringView id) const
{
    return !id.isEmpty() && m_factories.contains(id.toString());
}

} // namespace LfQuick
