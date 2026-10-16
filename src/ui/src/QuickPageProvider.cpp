#include <LfQuick/Ui/QuickPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUrl>

namespace LfQuick {
namespace {

class QuickPageInstance final : public IPageInstance
{
public:
    QuickPageInstance(QString pageId, QQuickItem *item, QQuickItem *host)
        : m_pageId(std::move(pageId))
        , m_item(item)
        , m_host(host)
    {
        if (m_item && m_host) {
            m_item->setSize(m_host->size());
            const QPointer<QQuickItem> item = m_item;
            const QPointer<QQuickItem> host = m_host;
            QObject::connect(m_host, &QQuickItem::widthChanged, m_item, [item, host] {
                if (item && host) item->setWidth(host->width());
            });
            QObject::connect(m_host, &QQuickItem::heightChanged, m_item, [item, host] {
                if (item && host) item->setHeight(host->height());
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
};

QUrl sourceUrl(const PageDescriptor &descriptor)
{
    const auto raw = descriptor.source.value(QStringLiteral("url"));
    if (raw.canConvert<QUrl>())
        return raw.toUrl();
    return QUrl(raw.toString());
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
    return m_engine && descriptor.provider == id() && sourceUrl(descriptor).isValid();
}

std::unique_ptr<IPageInstance> QuickPageProvider::create(const PageDescriptor &descriptor,
                                                          const QVariantMap &parameters)
{
    m_lastError.clear();
    if (!canCreate(descriptor)) {
        m_lastError = QStringLiteral("Invalid quick page descriptor or QQmlEngine");
        return {};
    }

    QQmlComponent component(m_engine, sourceUrl(descriptor));
    if (component.status() == QQmlComponent::Error) {
        m_lastError = component.errorString();
        return {};
    }

    QObject *object = component.createWithInitialProperties(parameters);
    if (!object) {
        m_lastError = component.errorString();
        return {};
    }

    auto *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        m_lastError = QStringLiteral("Quick page root must inherit QQuickItem");
        object->deleteLater();
        return {};
    }

    if (m_host) {
        item->setParent(m_host);
        item->setParentItem(m_host);
    }
    item->setVisible(false);

    return std::make_unique<QuickPageInstance>(descriptor.route, item, m_host);
}

void QuickPageProvider::setHost(QQuickItem *host)
{
    m_host = host;
}

} // namespace LfQuick
