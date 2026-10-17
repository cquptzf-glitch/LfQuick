#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/NavigationGuardController.h>
#include <LfQuick/Ui/QmlErrorCollector.h>

#include <LfQuick/Runtime/PageDescriptor.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QUrl>
#include <QVariantMap>

#include <functional>
#include <memory>

class QQmlEngine;

namespace LfQuick {

class INavigationGuard;
class IPageProvider;
class PageRouter;
class PageRegistry;
class PageProviderRegistry;
class QuickPageProvider;

class LFQUICKUI_EXPORT QuickRuntime : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY currentRouteChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged FINAL)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationChanged FINAL)
    Q_PROPERTY(int depth READ depth NOTIFY navigationChanged FINAL)
    Q_PROPERTY(int forwardDepth READ forwardDepth NOTIFY navigationChanged FINAL)
    Q_PROPERTY(QQuickItem* host READ host WRITE setHost NOTIFY hostChanged FINAL)
    Q_PROPERTY(QmlErrorCollector* errorCollector READ errorCollector CONSTANT FINAL)
    Q_PROPERTY(NavigationGuardController* navigationGuard READ navigationGuard CONSTANT FINAL)
    Q_PROPERTY(int maxRetainedLruPages READ maxRetainedLruPages WRITE setMaxRetainedLruPages NOTIFY cachePolicyChanged FINAL)
    Q_PROPERTY(qint64 maxRetainedLruWeight READ maxRetainedLruWeight WRITE setMaxRetainedLruWeight NOTIFY cachePolicyChanged FINAL)
    Q_PROPERTY(qint64 retainedLruWeight READ retainedLruWeight NOTIFY cacheStateChanged FINAL)
    Q_PROPERTY(int singletonCount READ singletonCount NOTIFY cacheStateChanged FINAL)

public:
    explicit QuickRuntime(QQmlEngine *engine, QObject *parent = nullptr);
    ~QuickRuntime() override;

    QString currentRoute() const;
    bool canGoBack() const;
    bool canGoForward() const;
    int depth() const;
    int forwardDepth() const;

    QQuickItem *host() const;
    void setHost(QQuickItem *host);

    QmlErrorCollector *errorCollector() const;
    NavigationGuardController *navigationGuard() const;

    int maxRetainedLruPages() const;
    void setMaxRetainedLruPages(int value);
    qint64 maxRetainedLruWeight() const;
    void setMaxRetainedLruWeight(qint64 value);
    qint64 retainedLruWeight() const;
    int singletonCount() const;

    bool registerProvider(std::shared_ptr<IPageProvider> provider);
    bool registerDescriptor(PageDescriptor descriptor);
    bool addNavigationGuard(std::shared_ptr<INavigationGuard> guard);
    void clearNavigationGuards();
    void setQuickActivationCallback(std::function<void()> callback);

    Q_INVOKABLE bool registerPage(const QString &route,
                                  const QUrl &source,
                                  int cachePolicy = static_cast<int>(PageCachePolicy::KeepAlive),
                                  bool useErrorFallback = true,
                                  qint64 cacheWeight = 1);
    Q_INVOKABLE bool unregisterPage(const QString &route);

    // Starts provider-specific background preparation without mutating route
    // history. Navigation remains synchronous and consumes the warmed candidate
    // when provider/source/parameters still match.
    Q_INVOKABLE bool prewarm(const QString &route,
                             const QVariantMap &parameters = {});
    Q_INVOKABLE void cancelPrewarm(const QString &route);

    Q_INVOKABLE bool push(const QString &route, const QVariantMap &parameters = {});
    Q_INVOKABLE bool replace(const QString &route, const QVariantMap &parameters = {});
    Q_INVOKABLE bool back();
    Q_INVOKABLE bool forward();
    Q_INVOKABLE void reset();

signals:
    void currentRouteChanged();
    void navigationChanged();
    void hostChanged();
    void cachePolicyChanged();
    void cacheStateChanged();
    void navigationFailed(const QString &route, const QString &message);
    void navigationBlocked(const QString &fromRoute,
                           const QString &toRoute,
                           const QString &reason);
    void cacheEntryEvicted(const QString &route);
    void prewarmFailed(const QString &route, const QString &message);
    void pageFallbackAttempted(const QString &route,
                               const QString &primaryProvider,
                               const QString &fallbackProvider,
                               const QString &primaryError);
    void pageFallbackActivated(const QString &route,
                               const QString &provider,
                               const QString &strategy);
    void pageFallbackFailed(const QString &route,
                            const QString &provider,
                            const QString &message);

private:
    PageDescriptor descriptorFor(const QString &route,
                                 const QUrl &source,
                                 int cachePolicy,
                                 bool useErrorFallback,
                                 qint64 cacheWeight) const;
    bool resolvePrewarmTarget(const QString &route,
                              PageDescriptor &descriptor,
                              std::shared_ptr<IPageProvider> &provider) const;

    QPointer<QQmlEngine> m_engine;
    QPointer<QQuickItem> m_host;
    QMetaObject::Connection m_hostDestroyedConnection;
    std::unique_ptr<PageRegistry> m_pages;
    std::unique_ptr<PageProviderRegistry> m_providers;
    std::unique_ptr<QmlErrorCollector> m_errorCollector;
    std::shared_ptr<QuickPageProvider> m_provider;
    std::unique_ptr<PageRouter> m_router;
    std::unique_ptr<NavigationGuardController> m_navigationGuardController;
};

} // namespace LfQuick
