#pragma once

#include <LfQuick/Platform/IInputSource.h>
#include <LfQuick/Platform/ScannerSession.h>

class QKeyEvent;

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT KeyboardWedgeScanner : public IInputSource
{
    Q_OBJECT
    Q_PROPERTY(ScannerSession* session READ session CONSTANT FINAL)

public:
    explicit KeyboardWedgeScanner(QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("keyboard-wedge"); }
    bool isEnabled() const override;
    void setEnabled(bool value) override;

    ScannerSession *session() const;

    Q_INVOKABLE bool handleKeyEvent(QKeyEvent *event);
    Q_INVOKABLE void simulateScan(const QString &value);

private:
    bool m_enabled = false;
    ScannerSession *m_session = nullptr;
};

} // namespace LfQuick
