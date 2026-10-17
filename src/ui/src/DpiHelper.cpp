#include <LfQuick/Ui/DpiHelper.h>

#include <QGuiApplication>
#include <QScreen>

namespace LfQuick {

DpiHelper::DpiHelper(QObject *parent)
    : QObject(parent)
{
    if (qGuiApp) {
        m_primaryScreenChangedConnection = connect(qGuiApp, &QGuiApplication::primaryScreenChanged,
                                                   this, [this](QScreen *) {
            reconnectPrimaryScreen();
            emit primaryScreenMetricsChanged();
        });
    }
    reconnectPrimaryScreen();
}

qreal DpiHelper::primaryLogicalDpi() const noexcept
{
    return logicalDpi(m_primaryScreen);
}

qreal DpiHelper::primaryDevicePixelRatio() const noexcept
{
    return devicePixelRatio(m_primaryScreen);
}

qreal DpiHelper::logicalDpi(QScreen *screen) const noexcept
{
    if (QScreen *resolved = resolveScreen(screen))
        return resolved->logicalDotsPerInch();
    return 96.0;
}

qreal DpiHelper::devicePixelRatio(QScreen *screen) const noexcept
{
    if (QScreen *resolved = resolveScreen(screen))
        return resolved->devicePixelRatio();
    return 1.0;
}

qreal DpiHelper::toPhysicalPixels(qreal logicalPixels, QScreen *screen) const noexcept
{
    return logicalPixels * devicePixelRatio(screen);
}

qreal DpiHelper::toLogicalPixels(qreal physicalPixels, QScreen *screen) const noexcept
{
    const qreal ratio = devicePixelRatio(screen);
    return ratio > 0.0 ? physicalPixels / ratio : physicalPixels;
}

qreal DpiHelper::scaleAgainst96Dpi(QScreen *screen) const noexcept
{
    return logicalDpi(screen) / 96.0;
}

QScreen *DpiHelper::resolveScreen(QScreen *screen) const noexcept
{
    if (screen)
        return screen;
    if (m_primaryScreen)
        return m_primaryScreen;
    return qGuiApp ? qGuiApp->primaryScreen() : nullptr;
}

void DpiHelper::reconnectPrimaryScreen()
{
    QObject::disconnect(m_logicalDpiChangedConnection);
    QObject::disconnect(m_physicalDpiChangedConnection);
    m_logicalDpiChangedConnection = {};
    m_physicalDpiChangedConnection = {};

    m_primaryScreen = qGuiApp ? qGuiApp->primaryScreen() : nullptr;
    if (!m_primaryScreen)
        return;

    m_logicalDpiChangedConnection = connect(m_primaryScreen, &QScreen::logicalDotsPerInchChanged,
                                            this, &DpiHelper::primaryScreenMetricsChanged);
    m_physicalDpiChangedConnection = connect(m_primaryScreen, &QScreen::physicalDotsPerInchChanged,
                                             this, &DpiHelper::primaryScreenMetricsChanged);
}

} // namespace LfQuick
