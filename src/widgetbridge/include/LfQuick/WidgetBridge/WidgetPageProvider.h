#pragma once

#include <LfQuick/Runtime/IPageProvider.h>
#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QHash>
#include <QPointer>
#include <QString>
#include <QStringView>

#include <functional>

class QWidget;

namespace LfQuick {

class LFQUICKWIDGETBRIDGE_EXPORT WidgetPageProvider final : public IPageProvider
{
public:
    using Factory = std::function<QWidget *(QWidget *parent, const QVariantMap &parameters)>;

    explicit WidgetPageProvider(QWidget *host = nullptr);

    QString id() const override;
    bool canCreate(const PageDescriptor &descriptor) const override;
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override;

    void setHost(QWidget *host);
    QWidget *host() const;

    bool registerFactory(QString id, Factory factory);
    bool unregisterFactory(QStringView id);
    bool hasFactory(QStringView id) const;

private:
    QPointer<QWidget> m_host;
    QHash<QString, Factory> m_factories;
};

} // namespace LfQuick
