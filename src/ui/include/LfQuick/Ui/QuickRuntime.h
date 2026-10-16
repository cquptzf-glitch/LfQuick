#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <LfQuick/Runtime/PageDescriptor.h>

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantMap>

#include <memory>

class QQmlEngine;
class QQuickItem;

namespace LfQuick {

class PageRouter;
class PageRegistry;
class PageProviderRegistry;
class QuickPageProvider;

class LFQUICKUI_EXPORT QuickRuntime : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY currentRouteChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged FINAL)
    Q_PROPERTY(int depth READ depth NOTIFY navigationChanged FINAL)
    Q_PROPERTY(QQuickItem* host READ host WRITE setHost NOTIFY hostChanged FINAL)

public:
    explicit QuickRuntime(QQmlEngine *engine, QObject *parent = nullptr);
    ~QuickRuntime() override;

    QString currentRoute() const;
    bool canGoBack() const;
    int depth() const;

    QQuickItem *host() const;
    void setHost(QQuickItem *host);

    Q_INVOKABLE bool registerPage(const QString &route,
                                  const QUrl &source,
                                  int cachePolicy = static_cast<int>(PageCachePolicy::KeepAlive));
    Q_INVOKABLE bool unregisterPage(const QString &route);

    Q_INVOKABLE bool push(const QString &route, const QVariantMap &parameters = {});
    Q_INVOKABLE bool replace(const QString &route, const QVariantMap &parameters = {});
    Q_INVOKABLE bool back();
    Q_INVOKABLE void reset();

signals:
    void currentRouteChanged();
    void navigationChanged();
    void hostChanged();
    void navigationFailed(const QString &route, const QString &message);

private:
    PageDescriptor descriptorFor(const QString &route, const QUrl &source, int cachePolicy) const;

    QPointer<QQmlEngine> m_engine;
    QPointer<QQuickItem> m_host;
    std::unique_ptr<PageRegistry> m_pages;
    std::unique_ptr<PageProviderRegistry> m_providers;
    std::shared_ptr<QuickPageProvider> m_provider;
    std::unique_ptr<PageRouter> m_router;
};

} // namespace LfQuick
