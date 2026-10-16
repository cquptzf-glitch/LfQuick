#pragma once

#include <LfQuick/Runtime/IPageProvider.h>

#include <QPointer>

#include <functional>

class QQmlEngine;
class QQuickItem;

namespace LfQuick {

class QmlErrorCollector;

class QuickPageProvider final : public IPageProvider
{
public:
    using ActivationCallback = std::function<void()>;

    explicit QuickPageProvider(QQmlEngine *engine, QQuickItem *host = nullptr);

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;
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
    void clearLastError();
    void setLastError(QString code, QString message, QVariantMap details = {});

    QPointer<QQmlEngine> m_engine;
    QPointer<QQuickItem> m_host;
    QPointer<QmlErrorCollector> m_errorCollector;
    ActivationCallback m_activationCallback;
    PageProviderError m_lastErrorInfo;
};

} // namespace LfQuick
