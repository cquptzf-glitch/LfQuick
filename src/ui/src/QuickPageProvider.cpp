#include <LfQuick/Ui/QuickPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Ui/LifetimeLease.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QmlObjectFactory.h>
#include <LfQuick/Ui/QmlStateSnapshot.h>

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
    bool isValid() const noexcept override { return !m_item.isNull(); }

    void activate() override
    {
        if (!m_item)
            return;
        if (m_activationCallback)
            m_activationCallback();

        if (!m_item)
            return;

        m_item->setVisible(true);
        QMetaObject::invokeMethod(m_item, "onPageActivated", Qt::DirectConnection);
    }

    void deactivate() override
    {
        if (!m_item)
            return;
        QMetaObject::invokeMethod(m_item, "onPageDeactivated", Qt::DirectConnection);
        if (m_item)
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

    bool updateParameters(const QVariantMap &parameters) override
    {
        if (!m_item)
            return false;

        bool accepted = true;
        for (auto it = parameters.cbegin(); it != parameters.cend(); ++it) {
            if (!m_item)
                return false;

            const QByteArray name = it.key().toUtf8();
            if (!m_item->setProperty(name.constData(), it.value()))
                accepted = false;
        }
        return accepted && !m_item.isNull();
    }

    QVariantMap saveState() const override
    {
        return m_item ? QmlStateSnapshot::capture(m_item) : QVariantMap{};
    }

    bool restoreState(const QVariantMap &state) override
    {
        if (!m_item)
            return state.isEmpty();
        if (state.isEmpty())
            return true;
        QmlStateSnapshot::restore(m_item, state);
        return true;
    }

    void destroy() override
    {
        if (!m_item)
            return;

        QPointer<QQuickItem> item = m_item;
        m_item = nullptr;

        const bool destroyScheduledImmediately = LifetimeLease::requestDestroy(item);
        if (destroyScheduledImmediately && item)
            item->setParentItem(nullptr);
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
    QObject *object = QmlObjectFactory::createSync(engine, url, properties, &error);
    if (!object)
        return nullptr;

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
    clearLastError();
    const QUrl primarySource = sourceUrl(descriptor);
    if (!canCreate(descriptor)) {
        const QString message = QStringLiteral("Invalid quick page descriptor or QQmlEngine");
        setLastError(QStringLiteral("quick.invalid-descriptor"), message,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("sourceUrl"), primarySource.toString()}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, primarySource, message,
                                     QStringLiteral("validate"));
        return {};
    }

    if (!m_host) {
        const QString message = QStringLiteral("Quick page host is not configured");
        setLastError(QStringLiteral("quick.host-missing"), message,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("sourceUrl"), primarySource.toString()},
                      {QStringLiteral("stage"), QStringLiteral("host")}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, primarySource, message,
                                     QStringLiteral("host"));
        return {};
    }

    QString primaryError;
    QQuickItem *item = createItem(m_engine, primarySource, parameters, primaryError);
    if (!item) {
        setLastError(QStringLiteral("quick.create-failed"), primaryError,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("sourceUrl"), primarySource.toString()},
                      {QStringLiteral("stage"), QStringLiteral("primary")}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, primarySource, primaryError,
                                     QStringLiteral("primary"));
        return {};
    }

    item->setParent(m_host);
    item->setParentItem(m_host);
    item->setVisible(false);

    return std::make_unique<QuickPageInstance>(descriptor.route,
                                                item,
                                                m_host,
                                                m_activationCallback);
}

std::unique_ptr<IPageInstance> QuickPageProvider::createErrorFallback(
    const PageDescriptor &descriptor,
    const QVariantMap &,
    const QString &primaryError)
{
    const QUrl fallbackSource = fallbackUrl(descriptor);
    if (!m_host) {
        const QString message = QStringLiteral("Quick page host is not configured");
        setLastError(QStringLiteral("quick.host-missing"), message,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("sourceUrl"), fallbackSource.toString()},
                      {QStringLiteral("stage"), QStringLiteral("fallback-host")}});
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, fallbackSource, message,
                                     QStringLiteral("fallback-host"));
        return {};
    }

    if (!fallbackSource.isValid() || fallbackSource.isEmpty()) {
        QString message = primaryError;
        if (!message.isEmpty())
            message += QLatin1Char('\n');
        message += QStringLiteral("Error fallback URL is not configured");
        setLastError(QStringLiteral("quick.fallback-not-configured"), message,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("stage"), QStringLiteral("fallback")}});
        return {};
    }

    QVariantMap fallbackProperties;
    fallbackProperties.insert(QStringLiteral("route"), descriptor.route);
    fallbackProperties.insert(QStringLiteral("errorMessage"), primaryError);

    QString fallbackError;
    QQuickItem *item = createItem(m_engine, fallbackSource, fallbackProperties, fallbackError);
    if (!item) {
        if (m_errorCollector)
            m_errorCollector->record(descriptor.route, fallbackSource, fallbackError,
                                     QStringLiteral("fallback"));
        QString message = primaryError;
        if (!message.isEmpty())
            message += QLatin1Char('\n');
        message += QStringLiteral("Fallback failed: ") + fallbackError;
        setLastError(QStringLiteral("quick.fallback-create-failed"), message,
                     {{QStringLiteral("route"), descriptor.route},
                      {QStringLiteral("sourceUrl"), fallbackSource.toString()},
                      {QStringLiteral("primaryError"), primaryError},
                      {QStringLiteral("fallbackError"), fallbackError},
                      {QStringLiteral("stage"), QStringLiteral("fallback")}});
        return {};
    }

    item->setParent(m_host);
    item->setParentItem(m_host);
    item->setVisible(false);

    setLastError(QStringLiteral("quick.primary-failed-fallback-active"), primaryError,
                 {{QStringLiteral("route"), descriptor.route},
                  {QStringLiteral("fallbackUrl"), fallbackSource.toString()}});
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

void QuickPageProvider::clearLastError()
{
    m_lastErrorInfo = {};
}

void QuickPageProvider::setLastError(QString code, QString message, QVariantMap details)
{
    m_lastErrorInfo.code = std::move(code);
    m_lastErrorInfo.message = std::move(message);
    m_lastErrorInfo.details = std::move(details);
}

} // namespace LfQuick
