#include <LfQuick/Ui/IWindowBackend.h>

namespace LfQuick {

bool QtWindowBackend::minimize(QWindow *window)
{
    if (!window)
        return false;
    window->showMinimized();
    return true;
}

bool QtWindowBackend::maximize(QWindow *window)
{
    if (!window)
        return false;
    window->showMaximized();
    return true;
}

bool QtWindowBackend::restore(QWindow *window)
{
    if (!window)
        return false;
    window->showNormal();
    return true;
}

bool QtWindowBackend::showFullScreen(QWindow *window)
{
    if (!window)
        return false;
    window->showFullScreen();
    return true;
}

bool QtWindowBackend::close(QWindow *window)
{
    return window ? window->close() : false;
}

bool QtWindowBackend::requestActivate(QWindow *window)
{
    if (!window)
        return false;
    window->requestActivate();
    return true;
}

bool QtWindowBackend::startSystemMove(QWindow *window)
{
    return window ? window->startSystemMove() : false;
}

bool QtWindowBackend::startSystemResize(QWindow *window, Qt::Edges edges)
{
    return window && edges != Qt::Edges{} ? window->startSystemResize(edges) : false;
}

bool QtWindowBackend::setAlwaysOnTop(QWindow *window, bool enabled)
{
    if (!window)
        return false;
    window->setFlag(Qt::WindowStaysOnTopHint, enabled);
    return true;
}

} // namespace LfQuick
