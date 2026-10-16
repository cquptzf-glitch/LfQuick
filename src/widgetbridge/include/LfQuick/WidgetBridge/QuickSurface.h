#pragma once

#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QElapsedTimer>
#include <QQuickWidget>
#include <QString>
#include <QUrl>

class QQmlEngine;
class QPaintEvent;

namespace LfQuick {

class LFQUICKWIDGETBRIDGE_EXPORT QuickSurface : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged FINAL)
    Q_PROPERTY(LoadState loadState READ loadState NOTIFY loadStateChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY loadFailed FINAL)
    Q_PROPERTY(qint64 loadDurationMs READ loadDurationMs NOTIFY metricsChanged FINAL)
    Q_PROPERTY(qint64 firstFrameMs READ firstFrameMs NOTIFY metricsChanged FINAL)

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

    void loadHost(const QUrl &url = QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/QuickHostRoot.qml")));
    Q_INVOKABLE void resetMetrics();

signals:
    void readyChanged();
    void loadStateChanged();
    void metricsChanged();
    void loadFailed(const QString &message);
    void firstFrameRendered(qint64 elapsedMs);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initialize();
    void handleStatusChanged(QQuickWidget::Status status);
    void setLoadState(LoadState state);

    QString m_lastError;
    LoadState m_loadState = LoadState::Idle;
    QElapsedTimer m_loadTimer;
    qint64 m_loadDurationMs = -1;
    qint64 m_firstFrameMs = -1;
    bool m_waitingForFirstFrame = false;
};

} // namespace LfQuick
