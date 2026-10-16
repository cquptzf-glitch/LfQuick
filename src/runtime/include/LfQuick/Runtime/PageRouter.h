#pragma once

#include <LfQuick/Core/Route.h>
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

public:
    explicit PageRouter(PageRegistry *pages,
                        PageProviderRegistry *providers,
                        QObject *parent = nullptr);
    ~PageRouter() override;

    QString currentRoute() const;
    bool canGoBack() const noexcept;
    int depth() const noexcept;
    QStringList history() const;

    RouteResult push(const RouteRequest &request);
    RouteResult replace(const RouteRequest &request);
    RouteResult back();
    void reset();

signals:
    void currentRouteChanged(const QString &route);
    void historyChanged();
    void navigationFailed(const QString &route, const QString &message);

private:
    struct Entry;

    RouteResult createEntry(const RouteRequest &request, Entry &entry);
    bool ensureInstance(Entry &entry);
    void deactivateCurrent(bool suspend);
    bool activate(Entry &entry);
    void destroy(Entry &entry);
    void notifyRouteChange();

    PageRegistry *m_pages = nullptr;
    PageProviderRegistry *m_providers = nullptr;
    std::vector<Entry> m_history;
};

} // namespace LfQuick
