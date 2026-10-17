#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQmlIncubator>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

// Drives QQmlIncubator work in small event-loop slices when an engine does not
// already have an incubation controller. If Qt (for example QQuickWindow) has
// installed its own controller, this class intentionally leaves it untouched.
class LFQUICKUI_EXPORT QmlIncubationScheduler final
    : public QObject
    , public QQmlIncubationController
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfIncubationScheduler)
    Q_PROPERTY(QQmlEngine* engine READ engine WRITE setEngine NOTIFY engineChanged FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(int budgetMs READ budgetMs WRITE setBudgetMs NOTIFY budgetChanged FINAL)
    Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalChanged FINAL)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged FINAL)
    Q_PROPERTY(bool installed READ isInstalled NOTIFY installedChanged FINAL)

public:
    explicit QmlIncubationScheduler(QObject *parent = nullptr);
    ~QmlIncubationScheduler() override;

    QQmlEngine *engine() const noexcept { return m_engine; }
    void setEngine(QQmlEngine *engine);

    bool isEnabled() const noexcept { return m_enabled; }
    void setEnabled(bool enabled);

    int budgetMs() const noexcept { return m_budgetMs; }
    void setBudgetMs(int value);

    int intervalMs() const noexcept { return m_intervalMs; }
    void setIntervalMs(int value);

    int pendingCount() const noexcept { return incubatingObjectCount(); }
    bool isInstalled() const noexcept;

    // Installs one scheduler as an engine-owned helper only when the engine has
    // no controller yet. Existing Qt/application controllers always win.
    static QmlIncubationScheduler *ensureForEngine(QQmlEngine *engine);

    Q_INVOKABLE bool install();
    Q_INVOKABLE void uninstall();
    Q_INVOKABLE void incubateNow(int budgetMs = -1);

signals:
    void engineChanged();
    void enabledChanged();
    void budgetChanged();
    void intervalChanged();
    void pendingCountChanged();
    void installedChanged();

protected:
    void timerEvent(QTimerEvent *event) override;
    void incubatingObjectCountChanged(int count) override;

private:
    void schedule();
    void cancelSchedule();
    void detachEngine();

    QPointer<QQmlEngine> m_engine;
    QMetaObject::Connection m_engineDestroyedConnection;
    int m_budgetMs = 4;
    int m_intervalMs = 1;
    int m_timerId = 0;
    bool m_enabled = true;
};

} // namespace LfQuick
