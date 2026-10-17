#pragma once

#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QHash>
#include <QString>
#include <QStringView>

#include <functional>
#include <memory>

class QWidget;

namespace LfQuick {

struct WidgetPageProviderState;

class LFQUICKWIDGETBRIDGE_EXPORT WidgetPageProvider final : public IPageProvider
{
public:
    using Factory = std::function<QWidget *(QWidget *parent, const QVariantMap &parameters)>;

    explicit WidgetPageProvider(QWidget *host = nullptr);
    ~WidgetPageProvider() override;

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;
    QString lastError() const override { return m_lastErrorInfo.message; }
    PageProviderError lastErrorInfo() const override { return m_lastErrorInfo; }

    void setHost(QWidget *host);
    QWidget *host() const;

    bool registerFactory(QString id, Factory factory);
    bool unregisterFactory(QStringView id);
    bool hasFactory(QStringView id) const;

private:
    std::shared_ptr<WidgetPageProviderState> m_state;
    QHash<QString, Factory> m_factories;
    PageProviderError m_lastErrorInfo;
};

} // namespace LfQuick
