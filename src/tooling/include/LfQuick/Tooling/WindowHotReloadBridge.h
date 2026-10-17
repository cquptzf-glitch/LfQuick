#pragma once

#include <LfQuick/Tooling/LfQuickToolingGlobal.h>
#include <LfQuick/Tooling/QmlHotReloadController.h>
#include <LfQuick/Ui/WindowRuntime.h>

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

// Connects file-level QML hot reload notifications to WindowRuntime's backing-
// preserving reload path. A watched logical source can be mapped to a local
// development file while the stable QQuickWindow/native surface remains alive.
class LFQUICKTOOLING_EXPORT WindowHotReloadBridge final : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(WindowRuntime* runtime READ runtime WRITE setRuntime NOTIFY runtimeChanged FINAL)
    Q_PROPERTY(QmlHotReloadController* controller READ controller WRITE setController NOTIFY controllerChanged FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(int watchedWindowCount READ watchedWindowCount NOTIFY watchedWindowsChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)

public:
    explicit WindowHotReloadBridge(QObject *parent = nullptr);
    ~WindowHotReloadBridge() override;

    WindowRuntime *runtime() const noexcept { return m_runtime; }
    void setRuntime(WindowRuntime *runtime);

    QmlHotReloadController *controller() const noexcept { return m_controller; }
    void setController(QmlHotReloadController *controller);

    bool isEnabled() const noexcept { return m_enabled; }
    void setEnabled(bool enabled);

    int watchedWindowCount() const noexcept { return m_watches.size(); }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool watchWindow(const QString &windowId,
                                 const QUrl &developmentSource = {});
    Q_INVOKABLE bool unwatchWindow(const QString &windowId);
    Q_INVOKABLE void clearWatches();

signals:
    void runtimeChanged();
    void controllerChanged();
    void enabledChanged();
    void watchedWindowsChanged();
    void lastErrorChanged();
    void reloadBatchFinished(qint64 revision, int attempted, int failed);

private:
    struct Watch {
        QUrl logicalSource;
        QUrl developmentSource;
    };

    void installSourceResolver();
    void releaseControllerWatches(QmlHotReloadController *controller);
    void acquireControllerWatches();
    void handleReloadRequested(const QStringList &logicalSources, qint64 revision);
    QUrl logicalSourceForWindow(const QString &windowId) const;
    void setLastError(const QString &error);

    QPointer<WindowRuntime> m_runtime;
    QPointer<QmlHotReloadController> m_controller;
    QHash<QString, Watch> m_watches;
    QMetaObject::Connection m_reloadConnection;
    QMetaObject::Connection m_controllerDestroyedConnection;
    QMetaObject::Connection m_runtimeDestroyedConnection;
    bool m_enabled = true;
    QString m_lastError;
};

} // namespace LfQuick
