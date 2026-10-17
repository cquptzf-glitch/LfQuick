#pragma once

#include <LfQuick/Core/Route.h>
#include <LfQuick/Runtime/INavigationGuard.h>
#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QHash>
#include <QObject>
#include <QStringList>

#include <memory>
#include <vector>

namespace LfQuick {

class IPageInstance;
class PageDescriptor;
class PageProviderRegistry;
class PageRegistry;

class LFQUICKRUNTIME_EXPORT PageRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY currentRouteChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged FINAL)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY historyChanged FINAL)
    Q_PROPERTY(int depth READ depth NOTIFY historyChanged FINAL)
    Q_PROPERTY(int forwardDepth READ forwardDepth NOTIFY historyChanged FINAL)
    Q_PROPERTY(int maxRetainedLruPages READ maxRetainedLruPages WRITE setMaxRetainedLruPages NOTIFY cachePolicyChanged FINAL)
    Q_PROPERTY(qint64 maxRetainedLruWeight READ maxRetainedLruWeight WRITE setMaxRetainedLruWeight NOTIFY cachePolicyChanged FINAL)
    Q_PROPERTY(qint64 retainedLruWeight READ retainedLruWeight NOTIFY cacheStateChanged FINAL)
    Q_PROPERTY(int singletonCount READ singletonCount NOTIFY cacheStateChanged FINAL)

public:
    explicit PageRouter(PageRegistry *pages,
                        PageProviderRegistry *providers,
                        QObject *parent = nullptr);
    ~PageRouter() override;

    QString currentRoute() const;
    bool canGoBack() const noexcept;
    bool canGoForward() const noexcept;
    int depth() const noexcept;
    int forwardDepth() const noexcept;
    QStringList history() const;
    QStringList forwardHistory() const;

    int maxRetainedLruPages() const noexcept;
    void setMaxRetainedLruPages(int value);
    qint64 maxRetainedLruWeight() const noexcept { return m_maxRetainedLruWeight; }
    void setMaxRetainedLruWeight(qint64 value);
    qint64 retainedLruWeight() const noexcept;
    int singletonCount() const noexcept { return m_singletons.size(); }

    bool addGuard(std::shared_ptr<INavigationGuard> guard);
    void clearGuards();

    RouteResult push(const RouteRequest &request);
    RouteResult replace(const RouteRequest &request);
    RouteResult back();
    RouteResult forward();
    void reset();

signals:
    void currentRouteChanged(const QString &route);
    void historyChanged();
    void cachePolicyChanged();
    void cacheStateChanged();
    void navigationFailed(const QString &route, const QString &message);
    void navigationBlocked(const QString &fromRoute,
                           const QString &toRoute,
                           const QString &reason);
    void cacheEntryEvicted(const QString &route);
    void singletonCreated(const QString &route);
    void singletonReused(const QString &route);
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
    struct Entry;

    RouteResult createEntry(const RouteRequest &request, Entry &entry);
    std::unique_ptr<IPageInstance> createInstance(const PageDescriptor &descriptor,
                                                   const QVariantMap &parameters,
                                                   QVariantMap &diagnostics);
    bool ensureInstance(Entry &entry);
    bool activate(Entry &entry);
    void deactivateCurrent(bool suspend);
    void destroy(Entry &entry);
    void clearForwardHistory();
    void destroySingletonCache();
    void touch(Entry &entry);
    void enforceLruLimit();
    bool passesGuards(NavigationAction action,
                      const QString &toRoute,
                      const QVariantMap &parameters,
                      RouteResult &failureResult);
    void notifyRouteChange();

    PageRegistry *m_pages = nullptr;
    PageProviderRegistry *m_providers = nullptr;
    std::vector<Entry> m_history;
    std::vector<Entry> m_forwardHistory;
    std::vector<std::shared_ptr<INavigationGuard>> m_guards;
    QHash<QString, std::shared_ptr<IPageInstance>> m_singletons;
    int m_maxRetainedLruPages = 3;
    qint64 m_maxRetainedLruWeight = 0;
    quint64 m_touchSequence = 0;
};

} // namespace LfQuick
