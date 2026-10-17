#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QtQmlIntegration/qqmlintegration.h>

class QScreen;

namespace LfQuick {

class LFQUICKUI_EXPORT DpiHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(qreal primaryLogicalDpi READ primaryLogicalDpi NOTIFY primaryScreenMetricsChanged FINAL)
    Q_PROPERTY(qreal primaryDevicePixelRatio READ primaryDevicePixelRatio NOTIFY primaryScreenMetricsChanged FINAL)

public:
    explicit DpiHelper(QObject *parent = nullptr);

    qreal primaryLogicalDpi() const noexcept;
    qreal primaryDevicePixelRatio() const noexcept;

    Q_INVOKABLE qreal logicalDpi(QScreen *screen = nullptr) const noexcept;
    Q_INVOKABLE qreal devicePixelRatio(QScreen *screen = nullptr) const noexcept;
    Q_INVOKABLE qreal toPhysicalPixels(qreal logicalPixels, QScreen *screen = nullptr) const noexcept;
    Q_INVOKABLE qreal toLogicalPixels(qreal physicalPixels, QScreen *screen = nullptr) const noexcept;
    Q_INVOKABLE qreal scaleAgainst96Dpi(QScreen *screen = nullptr) const noexcept;

signals:
    void primaryScreenMetricsChanged();

private:
    QScreen *resolveScreen(QScreen *screen) const noexcept;
    void reconnectPrimaryScreen();

    QPointer<QScreen> m_primaryScreen;
    QMetaObject::Connection m_primaryScreenChangedConnection;
    QMetaObject::Connection m_logicalDpiChangedConnection;
    QMetaObject::Connection m_physicalDpiChangedConnection;
};

} // namespace LfQuick
