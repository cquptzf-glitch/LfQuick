#pragma once

#include <LfQuick/Ui/IWindowBackend.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QWindow>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

namespace LfQuick {

class LFQUICKUI_EXPORT WindowController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged FINAL)
    Q_PROPERTY(bool active READ active NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool minimized READ minimized NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool maximized READ maximized NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool fullScreen READ fullScreen NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY stateChanged FINAL)

public:
    explicit WindowController(QObject *parent = nullptr);

    QWindow *window() const noexcept { return m_window; }
    void setWindow(QWindow *window);

    std::shared_ptr<IWindowBackend> backend() const { return m_backend; }
    void setBackend(std::shared_ptr<IWindowBackend> backend);

    bool active() const noexcept;
    bool minimized() const noexcept;
    bool maximized() const noexcept;
    bool fullScreen() const noexcept;
    bool alwaysOnTop() const noexcept;
    void setAlwaysOnTop(bool enabled);

    Q_INVOKABLE bool minimize();
    Q_INVOKABLE bool maximize();
    Q_INVOKABLE bool restore();
    Q_INVOKABLE bool toggleMaximized();
    Q_INVOKABLE bool toggleFullScreen();
    Q_INVOKABLE bool close();
    Q_INVOKABLE bool requestActivate();
    Q_INVOKABLE bool startSystemMove();
    Q_INVOKABLE bool startSystemResize(int edges);

signals:
    void windowChanged();
    void stateChanged();

private:
    void reconnectWindow();

    QPointer<QWindow> m_window;
    std::shared_ptr<IWindowBackend> m_backend;
    QMetaObject::Connection m_stateConnection;
    QMetaObject::Connection m_activeConnection;
    QMetaObject::Connection m_visibilityConnection;
    QMetaObject::Connection m_destroyedConnection;
};

} // namespace LfQuick
