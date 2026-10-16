#include <LfQuick/WidgetBridge/QuickSurface.h>

#include <QQmlEngine>

namespace LfQuick {

QuickSurface::QuickSurface(QWidget *parent)
    : QQuickWidget(parent)
{
    initialize();
}

QuickSurface::QuickSurface(QQmlEngine *engine, QWidget *parent)
    : QQuickWidget(engine, parent)
{
    initialize();
}

void QuickSurface::initialize()
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setClearColor(Qt::transparent);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QQuickWidget::statusChanged,
            this, &QuickSurface::handleStatusChanged);
}

bool QuickSurface::isReady() const noexcept
{
    return status() == QQuickWidget::Ready;
}

void QuickSurface::loadHost(const QUrl &url)
{
    m_lastError.clear();
    setSource(url);
}

void QuickSurface::handleStatusChanged(QQuickWidget::Status statusValue)
{
    if (statusValue == QQuickWidget::Error) {
        QStringList messages;
        for (const auto &error : errors())
            messages.push_back(error.toString());
        m_lastError = messages.join(QLatin1Char('\n'));
        emit loadFailed(m_lastError);
    }
    emit readyChanged();
}

} // namespace LfQuick
