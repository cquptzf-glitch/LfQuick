#pragma once

#include <LfQuick/Runtime/INavigationGuard.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/NavigationGuardState.h>

#include <QList>
#include <QObject>
#include <QVariantMap>

#include <memory>
#include <optional>

namespace LfQuick {

class PageRouter;

class LFQUICKUI_EXPORT NavigationGuardController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(NavigationGuardState* state READ state CONSTANT FINAL)
    Q_PROPERTY(bool hasPending READ hasPending NOTIFY pendingChanged FINAL)
    Q_PROPERTY(QString pendingFromRoute READ pendingFromRoute NOTIFY pendingChanged FINAL)
    Q_PROPERTY(QString pendingRoute READ pendingRoute NOTIFY pendingChanged FINAL)
    Q_PROPERTY(int pendingAction READ pendingAction NOTIFY pendingChanged FINAL)
    Q_PROPERTY(QVariantMap pendingParameters READ pendingParameters NOTIFY pendingChanged FINAL)
    Q_PROPERTY(PendingPolicy pendingPolicy READ pendingPolicy WRITE setPendingPolicy NOTIFY pendingPolicyChanged FINAL)
    Q_PROPERTY(int queuedRequestCount READ queuedRequestCount NOTIFY pendingChanged FINAL)
    Q_PROPERTY(int maxQueuedRequests READ maxQueuedRequests WRITE setMaxQueuedRequests NOTIFY pendingPolicyChanged FINAL)

public:
    enum class PendingPolicy {
        KeepFirst,
        ReplaceLatest,
        Queue
    };
    Q_ENUM(PendingPolicy)

    explicit NavigationGuardController(PageRouter *router, QObject *parent = nullptr);
    ~NavigationGuardController() override;

    NavigationGuardState *state() noexcept { return &m_state; }
    const NavigationGuardState *state() const noexcept { return &m_state; }

    bool hasPending() const noexcept { return m_pending.has_value(); }
    QString pendingFromRoute() const;
    QString pendingRoute() const;
    int pendingAction() const noexcept;
    QVariantMap pendingParameters() const;

    PendingPolicy pendingPolicy() const noexcept { return m_pendingPolicy; }
    void setPendingPolicy(PendingPolicy policy);
    int queuedRequestCount() const noexcept { return m_queue.size(); }
    int maxQueuedRequests() const noexcept { return m_maxQueuedRequests; }
    void setMaxQueuedRequests(int value);

    std::shared_ptr<INavigationGuard> guardHandle() const { return m_guard; }
    NavigationDecision evaluate(const NavigationContext &context);

    Q_INVOKABLE void cancelPending();
    Q_INVOKABLE bool confirmPending();
    Q_INVOKABLE void clearPendingRequests();

signals:
    void pendingChanged();
    void pendingPolicyChanged();
    void confirmationRequested(const QString &fromRoute,
                               const QString &toRoute,
                               const QString &reason);
    void hardBlocked(const QString &fromRoute,
                     const QString &toRoute,
                     const QString &reason);
    void navigationRequestQueued(const QString &fromRoute,
                                 const QString &toRoute,
                                 int queuedRequestCount);
    void navigationRequestDropped(const QString &fromRoute,
                                  const QString &toRoute,
                                  const QString &reason);

private:
    QString effectiveReason() const;
    bool sameNavigation(const NavigationContext &left,
                        const NavigationContext &right) const;
    bool isQueued(const NavigationContext &context) const;
    void setPending(const NavigationContext &context);
    void clearPending();
    void emitPendingPrompt();
    void promoteNextQueued();
    void dropQueuedRequests(const QString &reason);

    PageRouter *m_router = nullptr;
    NavigationGuardState m_state;
    std::shared_ptr<INavigationGuard> m_guard;
    std::optional<NavigationContext> m_pending;
    QList<NavigationContext> m_queue;
    std::optional<NavigationContext> m_bypassOnce;
    PendingPolicy m_pendingPolicy = PendingPolicy::KeepFirst;
    int m_maxQueuedRequests = 8;
    bool m_retryInProgress = false;
};

} // namespace LfQuick
