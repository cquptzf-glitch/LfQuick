#include <LfQuick/WidgetBridge/WidgetPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>

#include <QByteArray>
#include <QLayout>
#include <QPointer>
#include <QStackedWidget>
#include <QWidget>

#include <utility>

namespace LfQuick {

struct WidgetPageProviderState
{
    QPointer<QWidget> host;
    quint64 generation = 0;
};

namespace {

void invokeOptional(QObject *object, const char *method)
{
    if (!object || !method)
        return;

    const QByteArray signature = QByteArray(method) + QByteArrayLiteral("()");
    if (object->metaObject()->indexOfMethod(signature.constData()) < 0)
        return;

    QMetaObject::invokeMethod(object, method, Qt::DirectConnection);
}

class WidgetPageInstance final : public IPageInstance
{
public:
    WidgetPageInstance(QString pageId,
                       QWidget *widget,
                       std::shared_ptr<WidgetPageProviderState> state)
        : m_pageId(std::move(pageId))
        , m_widget(widget)
        , m_state(std::move(state))
        , m_host(m_state ? m_state->host : nullptr)
        , m_generation(m_state ? m_state->generation : 0)
    {
        if (!m_widget)
            return;

        if (m_host && m_widget->parentWidget() != m_host)
            m_widget->setParent(m_host);

        if (auto *stack = qobject_cast<QStackedWidget *>(m_host.data())) {
            if (stack->indexOf(m_widget) < 0)
                stack->addWidget(m_widget);
        } else if (m_host && m_host->layout()) {
            m_host->layout()->addWidget(m_widget);
        } else if (m_host) {
            m_widget->setGeometry(m_host->rect());
        }

        m_widget->hide();
    }

    ~WidgetPageInstance() override
    {
        destroy();
    }

    QString pageId() const override { return m_pageId; }
    bool isValid() const noexcept override
    {
        return !m_widget.isNull()
            && m_state
            && m_state->generation == m_generation
            && m_state->host == m_host;
    }

    void activate() override
    {
        if (!m_widget)
            return;

        if (auto *stack = qobject_cast<QStackedWidget *>(m_host.data()))
            stack->setCurrentWidget(m_widget);

        m_widget->show();
        m_widget->setFocus(Qt::OtherFocusReason);
        invokeOptional(m_widget, "onPageActivated");
    }

    void deactivate() override
    {
        if (!m_widget)
            return;

        invokeOptional(m_widget, "onPageDeactivated");
        m_widget->hide();
    }

    void suspend() override
    {
        invokeOptional(m_widget, "onPageSuspended");
    }

    void resume() override
    {
        invokeOptional(m_widget, "onPageResumed");
    }

    bool updateParameters(const QVariantMap &parameters) override
    {
        if (!m_widget)
            return false;

        m_widget->setProperty("lfquickPageParameters", parameters);
        const QByteArray signature("onPageParametersChanged(QVariantMap)");
        if (m_widget->metaObject()->indexOfMethod(signature.constData()) >= 0) {
            QMetaObject::invokeMethod(m_widget, "onPageParametersChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QVariantMap, parameters));
        }
        return true;
    }

    void destroy() override
    {
        if (!m_widget)
            return;

        if (auto *stack = qobject_cast<QStackedWidget *>(m_host.data()))
            stack->removeWidget(m_widget);
        else if (m_host && m_host->layout())
            m_host->layout()->removeWidget(m_widget);

        m_widget->deleteLater();
        m_widget = nullptr;
    }

private:
    QString m_pageId;
    QPointer<QWidget> m_widget;
    std::shared_ptr<WidgetPageProviderState> m_state;
    QPointer<QWidget> m_host;
    quint64 m_generation = 0;
};

QString factoryId(const PageDescriptor &descriptor)
{
    return descriptor.source.value(QStringLiteral("factory")).toString();
}

} // namespace

WidgetPageProvider::WidgetPageProvider(QWidget *host)
    : m_state(std::make_shared<WidgetPageProviderState>())
{
    m_state->host = host;
}

WidgetPageProvider::~WidgetPageProvider() = default;

QString WidgetPageProvider::id() const
{
    return QStringLiteral("widget");
}

bool WidgetPageProvider::canCreate(const PageDescriptor &descriptor) const
{
    return descriptor.provider == id() && hasFactory(factoryId(descriptor));
}

std::unique_ptr<IPageInstance> WidgetPageProvider::create(
    const PageDescriptor &descriptor,
    const QVariantMap &parameters)
{
    m_lastErrorInfo = {};
    const QString key = factoryId(descriptor);
    const auto it = m_factories.constFind(key);
    if (it == m_factories.cend()) {
        m_lastErrorInfo.code = QStringLiteral("widget.factory-not-found");
        m_lastErrorInfo.message = QStringLiteral("No widget factory registered for '%1'").arg(key);
        m_lastErrorInfo.details = {
            {QStringLiteral("route"), descriptor.route},
            {QStringLiteral("factory"), key}
        };
        return {};
    }

    QWidget *currentHost = m_state ? m_state->host.data() : nullptr;
    if (!currentHost) {
        m_lastErrorInfo.code = QStringLiteral("widget.host-missing");
        m_lastErrorInfo.message = QStringLiteral("Widget page host is not configured");
        m_lastErrorInfo.details = {
            {QStringLiteral("route"), descriptor.route},
            {QStringLiteral("factory"), key}
        };
        return {};
    }

    QWidget *widget = it.value()(currentHost, parameters);
    if (!widget) {
        m_lastErrorInfo.code = QStringLiteral("widget.factory-returned-null");
        m_lastErrorInfo.message = QStringLiteral("Widget factory '%1' returned null").arg(key);
        m_lastErrorInfo.details = {
            {QStringLiteral("route"), descriptor.route},
            {QStringLiteral("factory"), key}
        };
        return {};
    }

    widget->setProperty("lfquickPageParameters", parameters);
    return std::make_unique<WidgetPageInstance>(descriptor.route, widget, m_state);
}

void WidgetPageProvider::setHost(QWidget *host)
{
    if (!m_state)
        m_state = std::make_shared<WidgetPageProviderState>();
    if (m_state->host == host)
        return;

    m_state->host = host;
    ++m_state->generation;
}

QWidget *WidgetPageProvider::host() const
{
    return m_state ? m_state->host.data() : nullptr;
}

bool WidgetPageProvider::registerFactory(QString id, Factory factory)
{
    if (id.isEmpty() || !factory || m_factories.contains(id))
        return false;

    m_factories.insert(std::move(id), std::move(factory));
    return true;
}

bool WidgetPageProvider::unregisterFactory(QStringView id)
{
    return m_factories.remove(id.toString());
}

bool WidgetPageProvider::hasFactory(QStringView id) const
{
    return !id.isEmpty() && m_factories.contains(id.toString());
}

} // namespace LfQuick
