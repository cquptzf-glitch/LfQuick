#include <LfQuick/Io/DataStream.h>

#include <QIODevice>

#include <algorithm>
#include <utility>

namespace LfQuick {
namespace {

constexpr qint64 kReadChunkBytes = 64 * 1024;

qsizetype trailingDelimiterPrefixLength(const QByteArray &buffer,
                                        const QByteArray &delimiter)
{
    if (buffer.isEmpty() || delimiter.size() <= 1)
        return 0;

    const qsizetype maximum = std::min(buffer.size(), delimiter.size() - 1);
    for (qsizetype length = maximum; length > 0; --length) {
        if (buffer.endsWith(delimiter.first(length)))
            return length;
    }
    return 0;
}

qint64 discardExceptDelimiterPrefix(QByteArray &buffer,
                                    const QByteArray &delimiter)
{
    const qsizetype keep = trailingDelimiterPrefixLength(buffer, delimiter);
    const qsizetype drop = buffer.size() - keep;
    if (drop > 0)
        buffer.remove(0, drop);
    return static_cast<qint64>(drop);
}

} // namespace

void DataStreamParser::streamEnded(QByteArray &buffer)
{
    if (!buffer.isEmpty()) {
        emit chunk(buffer);
        emit read(QString::fromUtf8(buffer));
        buffer.clear();
    }
    emit finished();
}

DelimitedStreamParser::DelimitedStreamParser(QObject *parent)
    : DataStreamParser(parent)
{
}

void DelimitedStreamParser::setDelimiter(const QByteArray &delimiter)
{
    if (m_delimiter == delimiter)
        return;
    m_delimiter = delimiter;
    m_discardingOversizedRecord = false;
    emit delimiterChanged();
}

void DelimitedStreamParser::setMaxBufferedBytes(qint64 maxBufferedBytes)
{
    maxBufferedBytes = std::max<qint64>(0, maxBufferedBytes);
    if (m_maxBufferedBytes == maxBufferedBytes)
        return;
    m_maxBufferedBytes = maxBufferedBytes;
    emit maxBufferedBytesChanged();
}

void DelimitedStreamParser::parseBytes(QByteArray &incoming, QByteArray &buffer)
{
    if (incoming.isEmpty())
        return;

    buffer.append(incoming);
    incoming.clear();

    if (m_delimiter.isEmpty()) {
        m_discardingOversizedRecord = false;
        const QByteArray complete = std::exchange(buffer, QByteArray{});
        emit chunk(complete);
        emit read(QString::fromUtf8(complete));
        return;
    }

    if (m_discardingOversizedRecord) {
        const qsizetype recoveryIndex = buffer.indexOf(m_delimiter);
        if (recoveryIndex < 0) {
            // Keep only a suffix that can still become the beginning of a
            // delimiter in the next chunk. This preserves split delimiters
            // (for example "\r" + "\n") without retaining the oversized
            // record itself.
            const qint64 dropped = discardExceptDelimiterPrefix(buffer, m_delimiter);
            if (dropped > 0)
                emit bufferOverflow(dropped);
            return;
        }

        const qint64 dropped = recoveryIndex;
        buffer.remove(0, recoveryIndex + m_delimiter.size());
        if (dropped > 0)
            emit bufferOverflow(dropped);
        m_discardingOversizedRecord = false;
    }

    while (true) {
        const qsizetype index = buffer.indexOf(m_delimiter);
        if (index < 0)
            break;

        if (m_maxBufferedBytes > 0 && index > m_maxBufferedBytes) {
            const qint64 dropped = index;
            buffer.remove(0, index + m_delimiter.size());
            emit bufferOverflow(dropped);
            continue;
        }

        const QByteArray complete = buffer.left(index);
        buffer.remove(0, index + m_delimiter.size());
        emit chunk(complete);
        emit read(QString::fromUtf8(complete));
    }

    if (m_maxBufferedBytes > 0 && buffer.size() > m_maxBufferedBytes) {
        const qint64 dropped = discardExceptDelimiterPrefix(buffer, m_delimiter);
        m_discardingOversizedRecord = true;
        if (dropped > 0)
            emit bufferOverflow(dropped);
    }
}

void DelimitedStreamParser::streamEnded(QByteArray &buffer)
{
    if (m_discardingOversizedRecord) {
        const qint64 dropped = buffer.size();
        buffer.clear();
        m_discardingOversizedRecord = false;
        if (dropped > 0)
            emit bufferOverflow(dropped);
        emit finished();
        return;
    }

    if (!buffer.isEmpty()) {
        const QByteArray remaining = std::exchange(buffer, QByteArray{});
        emit chunk(remaining);
        emit read(QString::fromUtf8(remaining));
    }
    emit finished();
}

StreamCollector::StreamCollector(QObject *parent)
    : DataStreamParser(parent)
{
}

void StreamCollector::setWaitForEnd(bool waitForEnd)
{
    if (m_waitForEnd == waitForEnd)
        return;
    m_waitForEnd = waitForEnd;
    emit waitForEndChanged();
    if (!m_waitForEnd)
        emit dataChanged();
}

void StreamCollector::setMaxBytes(qint64 maxBytes)
{
    maxBytes = std::max<qint64>(0, maxBytes);
    if (m_maxBytes == maxBytes)
        return;
    m_maxBytes = maxBytes;
    if (m_maxBytes > 0 && m_data.size() > m_maxBytes) {
        const qint64 dropped = m_data.size() - m_maxBytes;
        m_data.remove(0, static_cast<qsizetype>(dropped));
        emit truncated(dropped);
        emit dataChanged();
    }
    emit maxBytesChanged();
}

void StreamCollector::parseBytes(QByteArray &incoming, QByteArray &buffer)
{
    if (!buffer.isEmpty()) {
        append(buffer);
        buffer.clear();
    }
    if (!incoming.isEmpty()) {
        append(incoming);
        incoming.clear();
    }
    if (!m_waitForEnd)
        emit dataChanged();
}

void StreamCollector::streamEnded(QByteArray &buffer)
{
    if (!buffer.isEmpty()) {
        append(buffer);
        buffer.clear();
    }
    if (m_waitForEnd)
        emit dataChanged();
    emit finished();
}

void StreamCollector::clear()
{
    if (m_data.isEmpty())
        return;
    m_data.clear();
    emit dataChanged();
}

void StreamCollector::append(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return;
    m_data.append(bytes);
    if (m_maxBytes <= 0 || m_data.size() <= m_maxBytes)
        return;

    const qint64 dropped = m_data.size() - m_maxBytes;
    m_data.remove(0, static_cast<qsizetype>(dropped));
    emit truncated(dropped);
}

DataStream::DataStream(QObject *parent)
    : QObject(parent)
{
}

DataStream::~DataStream()
{
    QObject::disconnect(m_parserDestroyedConnection);
    disconnectDevice();
}

void DataStream::setParser(DataStreamParser *parser)
{
    if (m_parser == parser)
        return;

    QObject::disconnect(m_parserDestroyedConnection);
    m_parserDestroyedConnection = {};

    const bool bufferChanged = !m_buffer.isEmpty();
    if (m_parser && !m_ended)
        m_parser->streamEnded(m_buffer);
    m_buffer.clear();
    m_parser = parser;
    m_ended = false;

    if (m_parser) {
        m_parserDestroyedConnection = connect(
            m_parser, &QObject::destroyed, this, [this] {
                const bool bufferChanged = !m_buffer.isEmpty();
                m_parser = nullptr;
                m_parserDestroyedConnection = {};
                m_buffer.clear();
                m_ended = false;
                emit parserChanged();
                if (bufferChanged)
                    emit bufferedBytesChanged();
            });
    }

    emit parserChanged();
    if (bufferChanged)
        emit bufferedBytesChanged();
    readAvailable();
}

QObject *DataStream::deviceObject() const
{
    return m_device.data();
}

void DataStream::setDeviceObject(QObject *device)
{
    attach(device);
}

bool DataStream::attach(QObject *device)
{
    auto *io = qobject_cast<QIODevice *>(device);
    if (!io && device)
        return false;
    attachDevice(io);
    return true;
}

void DataStream::attachDevice(QIODevice *device)
{
    if (m_device == device)
        return;

    if (m_device)
        readAvailable();
    disconnectDevice();
    if (m_parser && !m_ended)
        end();

    m_device = device;
    m_ended = false;
    if (m_device) {
        m_deviceConnections.push_back(connect(m_device, &QIODevice::readyRead,
                                              this, &DataStream::readAvailable));
        m_deviceConnections.push_back(connect(m_device, &QIODevice::aboutToClose,
                                              this, &DataStream::end));
        m_deviceConnections.push_back(connect(m_device, &QObject::destroyed,
                                              this, [this] {
            m_device = nullptr;
            m_deviceConnections.clear();
            end();
            emit deviceChanged();
        }));
    }
    emit deviceChanged();
    readAvailable();
}

void DataStream::detach()
{
    if (!m_device)
        return;
    readAvailable();
    disconnectDevice();
    m_device = nullptr;
    end();
    emit deviceChanged();
}

void DataStream::feed(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return;

    m_ended = false;
    emit bytesReceived(bytes);
    if (!m_parser)
        return;

    const qsizetype before = m_buffer.size();
    QByteArray incoming = bytes;
    m_parser->parseBytes(incoming, m_buffer);
    if (m_buffer.size() != before)
        emit bufferedBytesChanged();
}

void DataStream::end()
{
    if (m_ended)
        return;
    m_ended = true;

    if (!m_parser) {
        if (!m_buffer.isEmpty()) {
            m_buffer.clear();
            emit bufferedBytesChanged();
        }
        return;
    }

    const bool hadBuffer = !m_buffer.isEmpty();
    m_parser->streamEnded(m_buffer);
    if (hadBuffer || !m_buffer.isEmpty()) {
        m_buffer.clear();
        emit bufferedBytesChanged();
    }
}

void DataStream::readAvailable()
{
    if (!m_device || !m_device->isReadable())
        return;

    // Keep transient allocations bounded as well as the parser's persistent
    // buffer. A device may have accumulated a very large amount of readable
    // data before one readyRead delivery; readAll() would duplicate that whole
    // backlog before the parser had a chance to enforce its own limits.
    while (m_device && m_device->isReadable()) {
        const QByteArray bytes = m_device->read(kReadChunkBytes);
        if (bytes.isEmpty())
            break;
        feed(bytes);
        // feed() emits user-observable signals and may detach or destroy the
        // current device. The loop condition re-checks the guarded pointer on
        // the next iteration instead of dereferencing a stale device here.
    }
}

void DataStream::disconnectDevice()
{
    for (const auto &connection : std::as_const(m_deviceConnections))
        QObject::disconnect(connection);
    m_deviceConnections.clear();
}

} // namespace LfQuick
