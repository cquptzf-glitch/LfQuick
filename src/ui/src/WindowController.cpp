#include <LfQuick/Ui/WindowController.h>

namespace LfQuick {

WindowController::WindowController(QObject *parent)
    : QObject(parent)
{
}

void WindowController::setWindow(QWindow *window)
{
    if (m_window == window)
        return;

    QObject::disconnect(m_stateConnection);
    QObject::disconnect(m_activeConnection);
    QObject::disconnect(m_visibilityConnection);
    QObject::disconnect(m_destroyedConnection);
    m_stateConnection = {};
    m_activeConnection = {};
    m_visibilityConnection = {};
    m_destroyedConnection = {};

    m_window = window;
    reconnectWindow();
    emit windowChanged();
    emit stateChanged();
}

bool WindowController::active() const noexcept
{
    return m_window && m_window->isActive();
}

bool WindowController::minimized() const noexcept
{
    return m_window && m_window->windowState() == Qt::WindowMinimized;
}

bool WindowController::maximized() const noexcept
{
    return m_window && m_window->windowState() == Qt::WindowMaximized;
}

bool WindowController::fullScreen() const noexcept
{
    return m_window && m_window->windowState() == Qt::WindowFullScreen;
}

bool WindowController::alwaysOnTop() const noexcept
{
    return m_window && m_window->flags().testFlag(Qt::WindowStaysOnTopHint);
}

void WindowController::setAlwaysOnTop(bool enabled)
{
    if (!m_window || alwaysOnTop() == enabled)
        return;

    m_window->setFlag(Qt::WindowStaysOnTopHint, enabled);
    emit stateChanged();
}

bool WindowController::minimize()
{
    if (!m_window)
        return false;
    m_window->showMinimized();
    return true;
}

bool WindowController::maximize()
{
    if (!m_window)
        return false;
    m_window->showMaximized();
    return true;
}

bool WindowController::restore()
{
    if (!m_window)
        return false;
    m_window->showNormal();
    return true;
}

bool WindowController::toggleMaximized()
{
    return maximized() ? restore() : maximize();
}

bool WindowController::toggleFullScreen()
{
    if (!m_window)
        return false;
    if (fullScreen())
        m_window->showNormal();
    else
        m_window->showFullScreen();
    return true;
}

bool WindowController::close()
{
    return m_window ? m_window->close() : false;
}

bool WindowController::requestActivate()
{
    if (!m_window)
        return false;
    m_window->requestActivate();
    return true;
}

bool WindowController::startSystemMove()
{
    return m_window ? m_window->startSystemMove() : false;
}

bool WindowController::startSystemResize(int edges)
{
    if (!m_window || edges == 0)
        return false;

    const Qt::Edges requested(static_cast<Qt::Edge>(edges));
    return m_window->startSystemResize(requested);
}

void WindowController::reconnectWindow()
{
    if (!m_window)
        return;

    m_stateConnection = connect(m_window, &QWindow::windowStateChanged,
                                this, [this] { emit stateChanged(); });
    m_activeConnection = connect(m_window, &QWindow::activeChanged,
                                 this, &WindowController::stateChanged);
    m_visibilityConnection = connect(m_window, &QWindow::visibilityChanged,
                                     this, [this] { emit stateChanged(); });
    m_destroyedConnection = connect(m_window, &QObject::destroyed, this, [this] {
        m_window = nullptr;
        m_stateConnection = {};
        m_activeConnection = {};
        m_visibilityConnection = {};
        m_destroyedConnection = {};
        emit windowChanged();
        emit stateChanged();
    });
}

} // namespace LfQuick
