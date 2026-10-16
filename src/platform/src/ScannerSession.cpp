#include <LfQuick/Platform/ScannerSession.h>

namespace LfQuick {

ScannerSession::ScannerSession(QObject *parent)
    : QObject(parent)
{
}

bool ScannerSession::isActive() const
{
    return m_active;
}

int ScannerSession::timeoutMs() const
{
    return m_timeoutMs;
}

void ScannerSession::setTimeoutMs(int value)
{
    value = qBound(1, value, 2000);
    if (m_timeoutMs == value)
        return;
    m_timeoutMs = value;
    emit timeoutMsChanged();
}

int ScannerSession::minimumLength() const
{
    return m_minimumLength;
}

void ScannerSession::setMinimumLength(int value)
{
    value = qBound(1, value, 4096);
    if (m_minimumLength == value)
        return;
    m_minimumLength = value;
    emit minimumLengthChanged();
}

QString ScannerSession::prefix() const
{
    return m_prefix;
}

void ScannerSession::setPrefix(const QString &value)
{
    if (m_prefix == value)
        return;
    m_prefix = value;
    emit prefixChanged();
}

QString ScannerSession::suffix() const
{
    return m_suffix;
}

void ScannerSession::setSuffix(const QString &value)
{
    if (m_suffix == value)
        return;
    m_suffix = value;
    emit suffixChanged();
}

void ScannerSession::reset()
{
    const bool wasActive = m_active;
    m_active = false;
    m_buffer.clear();
    m_timer.invalidate();

    if (wasActive)
        emit activeChanged();
}

void ScannerSession::feed(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_timer.isValid() && m_timer.elapsed() > m_timeoutMs)
        m_buffer.clear();

    if (!m_active) {
        m_active = true;
        emit activeChanged();
    }

    m_buffer += text;
    m_timer.restart();
}

bool ScannerSession::finish()
{
    if (!m_active)
        return false;

    const bool accepted = validateAndEmit();
    const bool wasActive = m_active;
    m_active = false;
    m_buffer.clear();
    m_timer.invalidate();

    if (wasActive)
        emit activeChanged();

    return accepted;
}

void ScannerSession::simulateScan(const QString &value)
{
    if (value.isEmpty()) {
        emit scanRejected(value, QStringLiteral("empty"));
        return;
    }
    emit scanAccepted(value);
}

bool ScannerSession::validateAndEmit()
{
    const QString raw = m_buffer;
    if (raw.size() < m_minimumLength) {
        emit scanRejected(raw, QStringLiteral("too-short"));
        return false;
    }

    if (!m_prefix.isEmpty() && !raw.startsWith(m_prefix)) {
        emit scanRejected(raw, QStringLiteral("prefix-mismatch"));
        return false;
    }

    if (!m_suffix.isEmpty() && !raw.endsWith(m_suffix)) {
        emit scanRejected(raw, QStringLiteral("suffix-mismatch"));
        return false;
    }

    QString value = raw;
    if (!m_prefix.isEmpty())
        value.remove(0, m_prefix.size());
    if (!m_suffix.isEmpty())
        value.chop(m_suffix.size());

    if (value.size() < m_minimumLength) {
        emit scanRejected(raw, QStringLiteral("payload-too-short"));
        return false;
    }

    emit scanAccepted(value);
    return true;
}

} // namespace LfQuick
