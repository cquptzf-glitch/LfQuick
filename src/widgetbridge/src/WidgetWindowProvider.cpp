#include <LfQuick/WidgetBridge/WidgetWindowProvider.h>

#include <LfQuick/Runtime/IWindowInstance.h>

#include <QByteArray>
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QUuid>
#include <QWidget>

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

class WidgetWindowInstance final : public QObject, public IWindowInstance
{
public:
    WidgetWindowInstance(WindowInstanceId id,
                         QWidget *widget,
                         std::shared_ptr<WidgetWindowProviderState> state)
        : m_id(std::move(id)), m_widget(widget), m_state(std::move(state))
    {
        if (m_widget)
            m_widget->installEventFilter(this);
    }

    ~WidgetWindowInstance() override
    {
        if (m_widget)
            m_widget->removeEventFilter(this);
        if (m_state)
            m_state->widgets.remove(m_id);
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

    void close() override
    {
        if (!m_widget)
            return;

        invokeOptional(m_widget, "onWindowClosing");
        m_closingFromRouter = true;
        m_widget->close();
        m_widget->removeEventFilter(this);
        m_widget->deleteLater();
        m_widget = nullptr;
        m_closingFromRouter = false;
        if (m_state)
            m_state->widgets.remove(m_id);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_widget && event && event->type() == QEvent::Close
            && !m_closingFromRouter && m_closeRequestHandler) {
            event->ignore();
            m_closeRequestHandler();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    WindowInstanceId m_id;
    QPointer<QWidget> m_widget;
    std::shared_ptr<WidgetWindowProviderState> m_state;
    CloseRequestHandler m_closeRequestHandler;
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
    if (!request.ownerInstanceId.isEmpty())
        owner = m_state->widgets.value(request.ownerInstanceId);

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

    if (owner && widget->parentWidget() != owner)
        widget->setParent(owner, widget->windowFlags() | Qt::Window);
    widget->setAttribute(Qt::WA_DeleteOnClose, false);
    widget->setProperty("lfquickWindowParameters", request.parameters);

    const WindowInstanceId instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
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
    return m_factories.remove(id.toString()) > 0;
}

bool WidgetWindowProvider::hasFactory(QStringView id) const
{
    return !id.isEmpty() && m_factories.contains(id.toString());
}

} // namespace LfQuick
