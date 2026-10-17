#pragma once

#include <LfQuick/Platform/IInputSource.h>
#include <LfQuick/Platform/ScannerSession.h>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT MockScanner : public IInputSource
{
    Q_OBJECT
    Q_PROPERTY(ScannerSession* session READ session CONSTANT FINAL)

public:
    explicit MockScanner(QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("mock-scanner"); }
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;

    ScannerSession *session() const noexcept { return m_session; }

    Q_INVOKABLE void send(const QString &value);

private:
    bool m_enabled = true;
    ScannerSession *m_session = nullptr;
};

} // namespace LfQuick
