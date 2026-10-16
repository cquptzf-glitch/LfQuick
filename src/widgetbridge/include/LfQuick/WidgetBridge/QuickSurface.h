#pragma once

#include <QQuickWidget>
#include <QString>
#include <QUrl>

class QQmlEngine;

namespace LfQuick {

class QuickSurface : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY loadFailed FINAL)

public:
    explicit QuickSurface(QWidget *parent = nullptr);
    explicit QuickSurface(QQmlEngine *engine, QWidget *parent = nullptr);

    bool isReady() const noexcept;
    QString lastError() const { return m_lastError; }

    void loadHost(const QUrl &url = QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/QuickHostRoot.qml")));

signals:
    void readyChanged();
    void loadFailed(const QString &message);

private:
    void initialize();
    void handleStatusChanged(QQuickWidget::Status status);

    QString m_lastError;
};

} // namespace LfQuick
