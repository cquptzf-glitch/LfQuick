#include <LfQuick/Ui/WindowController.h>

#include <utility>

namespace LfQuick {

WindowController::WindowController(QObject *parent)
    : QObject(parent)
    , m_backend(std::make_shared<QtWindowBackend>())
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

void WindowController::setBackend(std::shared_ptr<IWindowBackend> backend)
{
    if (!backend)
        backend = std::make_shared<QtWindowBackend>();
    m_backend = std::move(backend);
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
    if (!m_window || !m_backend || alwaysOnTop() == enabled)
        return;

    if (m_backend->setAlwaysOnTop(m_window, enabled))
        emit stateChanged();
}

bool WindowController::minimize()
{
    return m_backend && m_backend->minimize(m_window);
}

bool WindowController::maximize()
{
    return m_backend && m_backend->maximize(m_window);
}

bool WindowController::restore()
{
    return m_backend && m_backend->restore(m_window);
}

bool WindowController::toggleMaximized()
{
    return maximized() ? restore() : maximize();
}

bool WindowController::toggleFullScreen()
{
    if (!m_window || !m_backend)
        return false;
    return fullScreen() ? m_backend->restore(m_window)
                        : m_backend->showFullScreen(m_window);
}

bool WindowController::close()
{
    return m_backend && m_backend->close(m_window);
}

bool WindowController::requestActivate()
{
    return m_backend && m_backend->requestActivate(m_window);
}

bool WindowController::startSystemMove()
{
    return m_backend && m_backend->startSystemMove(m_window);
}

bool WindowController::startSystemResize(int edges)
{
    if (!m_backend || !m_window || edges == 0)
        return false;
    return m_backend->startSystemResize(m_window, Qt::Edges::fromInt(edges));
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
