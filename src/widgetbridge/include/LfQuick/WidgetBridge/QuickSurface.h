#pragma once

#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QElapsedTimer>
#include <QQuickWidget>
#include <QSize>
#include <QString>
#include <QUrl>

class QEvent;
class QQmlEngine;
class QPaintEvent;
class QResizeEvent;

namespace LfQuick {

class LFQUICKWIDGETBRIDGE_EXPORT QuickSurface : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged FINAL)
    Q_PROPERTY(LoadState loadState READ loadState NOTIFY loadStateChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY loadFailed FINAL)
    Q_PROPERTY(qint64 loadDurationMs READ loadDurationMs NOTIFY metricsChanged FINAL)
    Q_PROPERTY(qint64 firstFrameMs READ firstFrameMs NOTIFY metricsChanged FINAL)
    Q_PROPERTY(qreal effectiveDevicePixelRatio READ effectiveDevicePixelRatio NOTIFY surfaceMetricsChanged FINAL)
    Q_PROPERTY(QSize surfaceSize READ surfaceSize NOTIFY surfaceMetricsChanged FINAL)
    Q_PROPERTY(QString screenName READ screenName NOTIFY surfaceMetricsChanged FINAL)
    Q_PROPERTY(quint64 resizeCount READ resizeCount NOTIFY surfaceMetricsChanged FINAL)

public:
    enum class LoadState {
        Idle,
        Loading,
        Ready,
        Error
    };
    Q_ENUM(LoadState)

    explicit QuickSurface(QWidget *parent = nullptr);
    explicit QuickSurface(QQmlEngine *engine, QWidget *parent = nullptr);

    bool isReady() const noexcept;
    LoadState loadState() const noexcept { return m_loadState; }
    QString lastError() const { return m_lastError; }
    qint64 loadDurationMs() const noexcept { return m_loadDurationMs; }
    qint64 firstFrameMs() const noexcept { return m_firstFrameMs; }
    qreal effectiveDevicePixelRatio() const noexcept { return devicePixelRatioF(); }
    QSize surfaceSize() const noexcept { return size(); }
    QString screenName() const;
    quint64 resizeCount() const noexcept { return m_resizeCount; }

    void loadHost(const QUrl &url = QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/QuickHostRoot.qml")));
    Q_INVOKABLE void resetMetrics();

signals:
    void readyChanged();
    void loadStateChanged();
    void metricsChanged();
    void surfaceMetricsChanged();
    void loadFailed(const QString &message);
    void firstFrameRendered(qint64 elapsedMs);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initialize();
    void handleStatusChanged(QQuickWidget::Status status);
    void setLoadState(LoadState state);

    QString m_lastError;
    LoadState m_loadState = LoadState::Idle;
    QElapsedTimer m_loadTimer;
    qint64 m_loadDurationMs = -1;
    qint64 m_firstFrameMs = -1;
    quint64 m_resizeCount = 0;
    bool m_waitingForFirstFrame = false;
};

} // namespace LfQuick
