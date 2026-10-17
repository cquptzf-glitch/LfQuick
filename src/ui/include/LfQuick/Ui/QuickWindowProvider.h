#pragma once

#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QPointer>

#include <memory>

class QQmlEngine;
class QQuickWindow;

namespace LfQuick {

class QmlErrorCollector;
struct QuickWindowProviderState;

// Window provider with a stable native QQuickWindow backing and reloadable QML
// content. WindowDescriptor::source["url"] must resolve to a QQuickItem root,
// not a Window root. The physical QQuickWindow survives logical reloads.
class LFQUICKUI_EXPORT QuickWindowProvider final : public IWindowProvider
{
public:
    explicit QuickWindowProvider(QQmlEngine *engine);
    ~QuickWindowProvider() override;

    QString id() const override;
    bool canCreate(const WindowDescriptor &descriptor) const override;
    std::unique_ptr<IWindowInstance> create(const WindowDescriptor &descriptor,
                                             const WindowOpenRequest &request) override;
    bool canAdoptBacking(const WindowDescriptor &descriptor,
                         const IWindowBacking &backing) const override;
    std::unique_ptr<IWindowInstance> createWithBacking(
        const WindowDescriptor &descriptor,
        const WindowOpenRequest &request,
        std::unique_ptr<IWindowBacking> &backing) override;

    QString lastError() const override { return m_lastErrorInfo.message; }
    WindowProviderError lastErrorInfo() const override { return m_lastErrorInfo; }

    void setErrorCollector(QmlErrorCollector *collector);

private:
    void clearLastError();
    void setLastError(QString code, QString message, QVariantMap details = {});

    QPointer<QQmlEngine> m_engine;
    QPointer<QmlErrorCollector> m_errorCollector;
    std::shared_ptr<QuickWindowProviderState> m_state;
    WindowProviderError m_lastErrorInfo;
};

} // namespace LfQuick
