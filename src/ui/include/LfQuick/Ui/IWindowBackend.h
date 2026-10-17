#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QWindow>

namespace LfQuick {

class LFQUICKUI_EXPORT IWindowBackend
{
public:
    virtual ~IWindowBackend() = default;

    virtual bool minimize(QWindow *window) = 0;
    virtual bool maximize(QWindow *window) = 0;
    virtual bool restore(QWindow *window) = 0;
    virtual bool showFullScreen(QWindow *window) = 0;
    virtual bool close(QWindow *window) = 0;
    virtual bool requestActivate(QWindow *window) = 0;
    virtual bool startSystemMove(QWindow *window) = 0;
    virtual bool startSystemResize(QWindow *window, Qt::Edges edges) = 0;
    virtual bool setAlwaysOnTop(QWindow *window, bool enabled) = 0;
};

class LFQUICKUI_EXPORT QtWindowBackend final : public IWindowBackend
{
public:
    bool minimize(QWindow *window) override;
    bool maximize(QWindow *window) override;
    bool restore(QWindow *window) override;
    bool showFullScreen(QWindow *window) override;
    bool close(QWindow *window) override;
    bool requestActivate(QWindow *window) override;
    bool startSystemMove(QWindow *window) override;
    bool startSystemResize(QWindow *window, Qt::Edges edges) override;
    bool setAlwaysOnTop(QWindow *window, bool enabled) override;
};

} // namespace LfQuick
