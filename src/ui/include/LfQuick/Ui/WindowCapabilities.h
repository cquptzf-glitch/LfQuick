#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QWindow>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT WindowCapabilities : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY capabilitiesChanged FINAL)
    Q_PROPERTY(QString platformName READ platformName NOTIFY capabilitiesChanged FINAL)
    Q_PROPERTY(bool framelessSupported READ framelessSupported CONSTANT FINAL)
    Q_PROPERTY(bool systemMoveSupported READ systemMoveSupported NOTIFY capabilitiesChanged FINAL)
    Q_PROPERTY(bool systemResizeSupported READ systemResizeSupported NOTIFY capabilitiesChanged FINAL)
    Q_PROPERTY(bool alwaysOnTopSupported READ alwaysOnTopSupported CONSTANT FINAL)
    Q_PROPERTY(bool transparencySupported READ transparencySupported CONSTANT FINAL)

public:
    explicit WindowCapabilities(QObject *parent = nullptr);

    QWindow *window() const noexcept { return m_window; }
    void setWindow(QWindow *window);

    QString platformName() const;
    bool framelessSupported() const noexcept { return true; }
    bool systemMoveSupported() const noexcept { return m_window != nullptr; }
    bool systemResizeSupported() const noexcept { return m_window != nullptr; }
    bool alwaysOnTopSupported() const noexcept { return true; }
    bool transparencySupported() const noexcept { return true; }

signals:
    void capabilitiesChanged();

private:
    QPointer<QWindow> m_window;
    QMetaObject::Connection m_destroyedConnection;
};

} // namespace LfQuick
