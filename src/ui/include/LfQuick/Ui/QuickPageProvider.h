#pragma once

#include <LfQuick/Runtime/IPageProvider.h>

#include <QPointer>

class QQmlEngine;
class QQuickItem;

namespace LfQuick {

class QuickPageProvider final : public IPageProvider
{
public:
    explicit QuickPageProvider(QQmlEngine *engine, QQuickItem *host = nullptr);

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;

    void setHost(QQuickItem *host);
    QString lastError() const { return m_lastError; }

private:
    QPointer<QQmlEngine> m_engine;
    QPointer<QQuickItem> m_host;
    QString m_lastError;
};

} // namespace LfQuick
