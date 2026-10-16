#pragma once

#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QHash>
#include <QString>
#include <QStringView>

#include <functional>
#include <memory>

class QWidget;

namespace LfQuick {

class LFQUICKWIDGETBRIDGE_EXPORT WidgetWindowProvider final : public IWindowProvider
{
public:
    using Factory = std::function<QWidget *(QWidget *owner, const QVariantMap &parameters)>;

    WidgetWindowProvider();
    ~WidgetWindowProvider() override;

    QString id() const override;
    bool canCreate(const WindowDescriptor &descriptor) const override;
    std::unique_ptr<IWindowInstance> create(const WindowDescriptor &descriptor,
                                             const WindowOpenRequest &request) override;
    QString lastError() const override { return m_lastErrorInfo.message; }
    WindowProviderError lastErrorInfo() const override { return m_lastErrorInfo; }

    bool registerFactory(QString id, Factory factory);
    bool unregisterFactory(QStringView id);
    bool hasFactory(QStringView id) const;

private:
    struct SharedState;
    std::shared_ptr<SharedState> m_state;
    QHash<QString, Factory> m_factories;
    WindowProviderError m_lastErrorInfo;
};

} // namespace LfQuick
