#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>
#include <LfQuick/Platform/ScannerSession.h>

#include <QObject>

class QKeyEvent;

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT KeyboardWedgeScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(ScannerSession* session READ session CONSTANT FINAL)

public:
    explicit KeyboardWedgeScanner(QObject *parent = nullptr);

    bool isEnabled() const;
    void setEnabled(bool value);

    ScannerSession *session() const;

    Q_INVOKABLE bool handleKeyEvent(QKeyEvent *event);
    Q_INVOKABLE void simulateScan(const QString &value);

signals:
    void enabledChanged();

private:
    bool m_enabled = false;
    ScannerSession *m_session = nullptr;
};

} // namespace LfQuick
