#include <LfQuick/Ui/WindowCapabilities.h>

#include <QGuiApplication>

namespace LfQuick {

WindowCapabilities::WindowCapabilities(QObject *parent)
    : QObject(parent)
{
}

void WindowCapabilities::setWindow(QWindow *window)
{
    if (m_window == window)
        return;

    QObject::disconnect(m_destroyedConnection);
    m_destroyedConnection = {};
    m_window = window;

    if (m_window) {
        m_destroyedConnection = connect(m_window, &QObject::destroyed, this, [this] {
            m_window = nullptr;
            m_destroyedConnection = {};
            emit capabilitiesChanged();
        });
    }

    emit capabilitiesChanged();
}

QString WindowCapabilities::platformName() const
{
    return QGuiApplication::platformName();
}

} // namespace LfQuick
