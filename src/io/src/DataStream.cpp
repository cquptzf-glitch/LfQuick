#include <LfQuick/Io/DataStream.h>

#include <QIODevice>

#include <algorithm>
#include <utility>

namespace LfQuick {

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
    emit delimiterChanged();
}

void DelimitedStreamParser::parseBytes(QByteArray &incoming, QByteArray &buffer)
{
    if (incoming.isEmpty())
        return;

    buffer.append(incoming);
    incoming.clear();

    if (m_delimiter.isEmpty()) {
        const QByteArray complete = std::exchange(buffer, {});
        emit chunk(complete);
        emit read(QString::fromUtf8(complete));
        return;
    }

    while (true) {
        const qsizetype index = buffer.indexOf(m_delimiter);
        if (index < 0)
            break;

        const QByteArray complete = buffer.left(index);
        buffer.remove(0, index + m_delimiter.size());
        emit chunk(complete);
        emit read(QString::fromUtf8(complete));
    }
}

void DelimitedStreamParser::streamEnded(QByteArray &buffer)
{
    if (!buffer.isEmpty()) {
        const QByteArray remaining = std::exchange(buffer, {});
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
    disconnectDevice();
}

void DataStream::setParser(DataStreamParser *parser)
{
    if (m_parser == parser)
        return;

    if (m_parser && !m_buffer.isEmpty())
        m_parser->streamEnded(m_buffer);
    const bool bufferChanged = !m_buffer.isEmpty();
    m_buffer.clear();
    m_parser = parser;
    emit parserChanged();
    if (bufferChanged)
        emit bufferedBytesChanged();
    readAvailable();
}

QObject *DataStream::deviceObject() const
{
    return m_device.data();
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

    disconnectDevice();
    m_device = device;
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
    const QByteArray bytes = m_device->readAll();
    if (!bytes.isEmpty())
        feed(bytes);
}

void DataStream::disconnectDevice()
{
    for (const auto &connection : std::as_const(m_deviceConnections))
        QObject::disconnect(connection);
    m_deviceConnections.clear();
}

} // namespace LfQuick
