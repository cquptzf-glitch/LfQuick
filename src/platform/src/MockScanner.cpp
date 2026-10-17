#include <LfQuick/Platform/MockScanner.h>

namespace LfQuick {

MockScanner::MockScanner(QObject *parent)
    : IInputSource(parent)
    , m_session(new ScannerSession(this))
{
    connect(m_session, &ScannerSession::scanAccepted, this, [this](const QString &value) {
        emit inputReceived(value, {
            {QStringLiteral("kind"), QStringLiteral("scanner")},
            {QStringLiteral("sourceId"), id()},
            {QStringLiteral("mock"), true}
        });
    });

    connect(m_session, &ScannerSession::scanRejected, this,
            [this](const QString &rawValue, const QString &reason) {
        emit inputRejected(reason, {
            {QStringLiteral("kind"), QStringLiteral("scanner")},
            {QStringLiteral("sourceId"), id()},
            {QStringLiteral("mock"), true},
            {QStringLiteral("rawLength"), rawValue.size()}
        });
    });
}

void MockScanner::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    if (!m_enabled)
        m_session->reset();
    emit enabledChanged();
}

void MockScanner::send(const QString &value)
{
    if (!m_enabled)
        return;
    m_session->simulateScan(value);
}

} // namespace LfQuick
