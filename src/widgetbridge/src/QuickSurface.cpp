#include <LfQuick/WidgetBridge/QuickSurface.h>

#include <QQmlEngine>
#include <QPaintEvent>

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
    m_loadDurationMs = -1;
    m_firstFrameMs = -1;
    m_waitingForFirstFrame = true;
    m_loadTimer.restart();
    setLoadState(LoadState::Loading);
    emit metricsChanged();
    setSource(url);
}

void QuickSurface::resetMetrics()
{
    m_loadDurationMs = -1;
    m_firstFrameMs = -1;
    m_waitingForFirstFrame = false;
    m_loadTimer.invalidate();
    emit metricsChanged();
}

void QuickSurface::paintEvent(QPaintEvent *event)
{
    QQuickWidget::paintEvent(event);

    if (!m_waitingForFirstFrame || status() != QQuickWidget::Ready || !m_loadTimer.isValid())
        return;

    m_waitingForFirstFrame = false;
    m_firstFrameMs = m_loadTimer.elapsed();
    emit metricsChanged();
    emit firstFrameRendered(m_firstFrameMs);
}

void QuickSurface::handleStatusChanged(QQuickWidget::Status statusValue)
{
    switch (statusValue) {
    case QQuickWidget::Null:
        setLoadState(LoadState::Idle);
        break;
    case QQuickWidget::Loading:
        setLoadState(LoadState::Loading);
        break;
    case QQuickWidget::Ready:
        if (m_loadTimer.isValid())
            m_loadDurationMs = m_loadTimer.elapsed();
        setLoadState(LoadState::Ready);
        emit metricsChanged();
        break;
    case QQuickWidget::Error: {
        if (m_loadTimer.isValid())
            m_loadDurationMs = m_loadTimer.elapsed();
        m_waitingForFirstFrame = false;

        QStringList messages;
        for (const auto &error : errors())
            messages.push_back(error.toString());
        m_lastError = messages.join(QLatin1Char('\n'));

        setLoadState(LoadState::Error);
        emit metricsChanged();
        emit loadFailed(m_lastError);
        break;
    }
    }

    emit readyChanged();
}

void QuickSurface::setLoadState(LoadState state)
{
    if (m_loadState == state)
        return;
    m_loadState = state;
    emit loadStateChanged();
}

} // namespace LfQuick
