#pragma once

#include <LfQuick/Runtime/IPageProvider.h>

#include <QHash>
#include <QPointer>
#include <QVariantMap>

#include <functional>
#include <memory>

class QQmlEngine;
class QQuickItem;

namespace LfQuick {

class QmlErrorCollector;
class QmlObjectFactory;
class QmlObjectLoadHandle;

class QuickPageProvider final : public IPageProvider
{
public:
    using ActivationCallback = std::function<void()>;

    explicit QuickPageProvider(QQmlEngine *engine, QQuickItem *host = nullptr);
    ~QuickPageProvider() override;

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;
    bool prewarm(const PageDescriptor &descriptor,
                 const QVariantMap &parameters) override;
    void cancelPrewarm(QStringView route) override;

    QString lastError() const override { return m_lastErrorInfo.message; }
    PageProviderError lastErrorInfo() const override { return m_lastErrorInfo; }
    std::unique_ptr<IPageInstance> createErrorFallback(
        const PageDescriptor &descriptor,
        const QVariantMap &parameters,
        const QString &primaryError) override;

    void setHost(QQuickItem *host);
    void setErrorCollector(QmlErrorCollector *collector);
    void setActivationCallback(ActivationCallback callback);

private:
    struct PrewarmEntry {
        QVariantMap parameters;
        QPointer<QmlObjectLoadHandle> handle;
    };

    QQuickItem *takePrewarmedItem(const PageDescriptor &descriptor,
                                  const QVariantMap &parameters,
                                  QString &error);
    void discardPrewarm(const QString &route);
    void clearLastError();
    void setLastError(QString code, QString message, QVariantMap details = {});

    QPointer<QQmlEngine> m_engine;
    QPointer<QQuickItem> m_host;
    QPointer<QmlErrorCollector> m_errorCollector;
    ActivationCallback m_activationCallback;
    std::unique_ptr<QmlObjectFactory> m_factory;
    QHash<QString, PrewarmEntry> m_prewarms;
    PageProviderError m_lastErrorInfo;
};

} // namespace LfQuick
