#pragma once

#include <LfQuick/Core/Route.h>
#include <LfQuick/Runtime/INavigationGuard.h>
#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QObject>
#include <QStringList>

#include <memory>
#include <vector>

namespace LfQuick {

class IPageInstance;
class PageProviderRegistry;
class PageRegistry;

class LFQUICKRUNTIME_EXPORT PageRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY currentRouteChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged FINAL)
    Q_PROPERTY(int depth READ depth NOTIFY historyChanged FINAL)
    Q_PROPERTY(int maxRetainedLruPages READ maxRetainedLruPages WRITE setMaxRetainedLruPages NOTIFY cachePolicyChanged FINAL)

public:
    explicit PageRouter(PageRegistry *pages,
                        PageProviderRegistry *providers,
                        QObject *parent = nullptr);
    ~PageRouter() override;

    QString currentRoute() const;
    bool canGoBack() const noexcept;
    int depth() const noexcept;
    QStringList history() const;

    int maxRetainedLruPages() const noexcept;
    void setMaxRetainedLruPages(int value);

    bool addGuard(std::shared_ptr<INavigationGuard> guard);
    void clearGuards();

    RouteResult push(const RouteRequest &request);
    RouteResult replace(const RouteRequest &request);
    RouteResult back();
    void reset();

signals:
    void currentRouteChanged(const QString &route);
    void historyChanged();
    void cachePolicyChanged();
    void navigationFailed(const QString &route, const QString &message);
    void navigationBlocked(const QString &fromRoute,
                           const QString &toRoute,
                           const QString &reason);
    void cacheEntryEvicted(const QString &route);

private:
    struct Entry;

    RouteResult createEntry(const RouteRequest &request, Entry &entry);
    bool ensureInstance(Entry &entry);
    bool activate(Entry &entry);
    void deactivateCurrent(bool suspend);
    void destroy(Entry &entry);
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
    std::vector<std::shared_ptr<INavigationGuard>> m_guards;
    int m_maxRetainedLruPages = 3;
    quint64 m_touchSequence = 0;
};

} // namespace LfQuick
