#include <LfQuick/Ui/QuickPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Ui/QmlErrorCollector.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUrl>

#include <utility>

namespace LfQuick {
namespace {

class QuickPageInstance final : public IPageInstance
{
public:
    QuickPageInstance(QString pageId,
                      QQuickItem *item,
                      QQuickItem *host,
                      QuickPageProvider::ActivationCallback activationCallback)
        : m_pageId(std::move(pageId))
        , m_item(item)
        , m_host(host)
        , m_activationCallback(std::move(activationCallback))
    {
        if (m_item && m_host) {
            m_item->setSize(m_host->size());
            const QPointer<QQuickItem> item = m_item;
            const QPointer<QQuickItem> host = m_host;
            QObject::connect(m_host, &QQuickItem::widthChanged, m_item, [item, host] {
                if (item && host)
                    item->setWidth(host->width());
            });
            QObject::connect(m_host, &QQuickItem::heightChanged, m_item, [item, host] {
                if (item && host)
                    item->setHeight(host->height());
            });
        }
    }

    ~QuickPageInstance() override
    {
        destroy();
    }

    QString pageId() const override { return m_pageId; }

    void activate() override
    {
        if (!m_item)
            return;
        if (m_activationCallback)
            m_activationCallback();
        m_item->setVisible(true);
        QMetaObject::invokeMethod(m_item, "onPageActivated", Qt::DirectConnection);
    }

    void deactivate() override
    {
        if (!m_item)
            return;
        QMetaObject::invokeMethod(m_item, "onPageDeactivated", Qt::DirectConnection);
        m_item->setVisible(false);
    }

    void suspend() override
    {
        if (m_item)
            QMetaObject::invokeMethod(m_item, "onPageSuspended", Qt::DirectConnection);
    }

    void resume() override
    {
        if (m_item)
            QMetaObject::invokeMethod(m_item, "onPageResumed", Qt::DirectConnection);
    }

    void destroy() override
    {
        if (!m_item)
            return;
        m_item->setParentItem(nullptr);
        m_item->deleteLater();
        m_item = nullptr;
    }

private:
    QString m_pageId;
    QPointer<QQuickItem> m_item;
    QPointer<QQuickItem> m_host;
    QuickPageProvider::ActivationCallback m_activationCallback;
};

QUrl sourceUrl(const PageDescriptor &descriptor)
{
    const auto raw = descriptor.source.value(QStringLiteral("url"));
    if (raw.canConvert<QUrl>())
        return raw.toUrl();
    return QUrl(raw.toString());
}

QUrl fallbackUrl(const PageDescriptor &descriptor)
{
    const auto raw = descriptor.source.value(QStringLiteral("fallbackUrl"));
    if (raw.canConvert<QUrl>())
        return raw.toUrl();
    return QUrl(raw.toString());
}

QQuickItem *createItem(QQmlEngine *engine,
                       const QUrl &url,
                       const QVariantMap &properties,
                       QString &error)
{
    if (!engine || url.isEmpty() || !url.isValid()) {
        error = QStringLiteral("Invalid QML source URL");
        return nullptr;
    }

    QQmlComponent component(engine, url);
    if (component.status() == QQmlComponent::Error) {
        error = component.errorString();
        return nullptr;
    }

    QObject *object = component.createWithInitialProperties(properties);
    if (!object) {
        error = component.errorString();
        return nullptr;
    }

    auto *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        error = QStringLiteral("Quick page root must inherit QQuickItem");
        object->deleteLater();
        return nullptr;
    }

    return item;
}

} // namespace

QuickPageProvider::QuickPageProvider(QQmlEngine *engine, QQuickItem *host)
    : m_engine(engine)
    , m_host(host)
{
}

QString QuickPageProvider::id() const
{
    return QStringLiteral("quick");
}

bool QuickPageProvider::canCreate(const PageDescriptor &descriptor) const
{
    const QUrl url = sourceUrl(descriptor);
    return m_engine && descriptor.provider == id() && url.isValid() && !url.isEmpty();
}

std::unique_ptr<IPageInstance> QuickPageProvider::create(const PageDescriptor &descriptor,
                                                          const QVariantMap &parameters)
{
    m_lastError.clear();
    if (!canCreate(descriptor)) {
        m_lastError = QStringLiteral("Invalid quick page descriptor or QQmlEngine");
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, sourceUrl(descriptor), m_lastError,
                                     QStringLiteral("validate"));
        return {};
    }

    const QUrl primarySource = sourceUrl(descriptor);
    QString primaryError;
    QQuickItem *item = createItem(m_engine, primarySource, parameters, primaryError);

    if (!item) {
        m_lastError = primaryError;
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, primarySource, primaryError,
                                     QStringLiteral("primary"));

        const QUrl fallbackSource = fallbackUrl(descriptor);
        if (descriptor.fallbackPolicy == FallbackPolicy::ErrorPage
            && fallbackSource.isValid() && !fallbackSource.isEmpty()) {
            // Do not forward the failed business page's initial properties to the
            // generic fallback page. Unknown properties would make the fallback
            // fail for exactly the pages where diagnostics are most important.
            QVariantMap fallbackProperties;
            fallbackProperties.insert(QStringLiteral("route"), descriptor.route);
            fallbackProperties.insert(QStringLiteral("errorMessage"), primaryError);

            QString fallbackError;
            item = createItem(m_engine, fallbackSource, fallbackProperties, fallbackError);
            if (!item) {
                if (m_errorCollector)
                    m_errorCollector->record(descriptor.route, fallbackSource, fallbackError,
                                             QStringLiteral("fallback"));
                m_lastError += QStringLiteral("\nFallback failed: ") + fallbackError;
            }
        }
    }

    if (!item)
        return {};

    if (m_host) {
        item->setParent(m_host);
        item->setParentItem(m_host);
    }
    item->setVisible(false);

    return std::make_unique<QuickPageInstance>(descriptor.route,
                                                item,
                                                m_host,
                                                m_activationCallback);
}

void QuickPageProvider::setHost(QQuickItem *host)
{
    m_host = host;
}

void QuickPageProvider::setErrorCollector(QmlErrorCollector *collector)
{
    m_errorCollector = collector;
}

void QuickPageProvider::setActivationCallback(ActivationCallback callback)
{
    m_activationCallback = std::move(callback);
}

} // namespace LfQuick
