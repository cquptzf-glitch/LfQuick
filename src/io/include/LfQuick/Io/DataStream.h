#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QByteArray>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

class QIODevice;

namespace LfQuick {

class LFQUICKIO_EXPORT DataStreamParser : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfDataStreamParser)
    QML_UNCREATABLE("LfDataStreamParser is an abstract base class")

public:
    explicit DataStreamParser(QObject *parent = nullptr) : QObject(parent) {}
    ~DataStreamParser() override = default;

    virtual void parseBytes(QByteArray &incoming, QByteArray &buffer) = 0;
    virtual void streamEnded(QByteArray &buffer);

signals:
    void chunk(const QByteArray &data);
    void read(const QString &text);
    void finished();
};

class LFQUICKIO_EXPORT DelimitedStreamParser final : public DataStreamParser
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfDelimitedStreamParser)
    Q_PROPERTY(QByteArray delimiter READ delimiter WRITE setDelimiter NOTIFY delimiterChanged FINAL)

public:
    explicit DelimitedStreamParser(QObject *parent = nullptr);

    QByteArray delimiter() const { return m_delimiter; }
    void setDelimiter(const QByteArray &delimiter);

    void parseBytes(QByteArray &incoming, QByteArray &buffer) override;
    void streamEnded(QByteArray &buffer) override;

signals:
    void delimiterChanged();

private:
    QByteArray m_delimiter = QByteArrayLiteral("\n");
};

class LFQUICKIO_EXPORT StreamCollector final : public DataStreamParser
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfStreamCollector)
    Q_PROPERTY(QByteArray data READ data NOTIFY dataChanged FINAL)
    Q_PROPERTY(QString text READ text NOTIFY dataChanged FINAL)
    Q_PROPERTY(bool waitForEnd READ waitForEnd WRITE setWaitForEnd NOTIFY waitForEndChanged FINAL)
    Q_PROPERTY(qint64 maxBytes READ maxBytes WRITE setMaxBytes NOTIFY maxBytesChanged FINAL)

public:
    explicit StreamCollector(QObject *parent = nullptr);

    QByteArray data() const { return m_data; }
    QString text() const { return QString::fromUtf8(m_data); }
    bool waitForEnd() const noexcept { return m_waitForEnd; }
    void setWaitForEnd(bool waitForEnd);
    qint64 maxBytes() const noexcept { return m_maxBytes; }
    void setMaxBytes(qint64 maxBytes);

    void parseBytes(QByteArray &incoming, QByteArray &buffer) override;
    void streamEnded(QByteArray &buffer) override;
    Q_INVOKABLE void clear();

signals:
    void dataChanged();
    void waitForEndChanged();
    void maxBytesChanged();
    void truncated(qint64 droppedBytes);

private:
    void append(const QByteArray &bytes);

    QByteArray m_data;
    bool m_waitForEnd = true;
    qint64 m_maxBytes = 4 * 1024 * 1024;
};

class LFQUICKIO_EXPORT DataStream final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfDataStream)
    Q_PROPERTY(DataStreamParser* parser READ parser WRITE setParser NOTIFY parserChanged FINAL)
    Q_PROPERTY(QObject* device READ deviceObject WRITE attach NOTIFY deviceChanged FINAL)
    Q_PROPERTY(bool attached READ attached NOTIFY deviceChanged FINAL)
    Q_PROPERTY(qint64 bufferedBytes READ bufferedBytes NOTIFY bufferedBytesChanged FINAL)

public:
    explicit DataStream(QObject *parent = nullptr);
    ~DataStream() override;

    DataStreamParser *parser() const { return m_parser; }
    void setParser(DataStreamParser *parser);

    QObject *deviceObject() const;
    bool attached() const noexcept { return !m_device.isNull(); }
    qint64 bufferedBytes() const noexcept { return m_buffer.size(); }

    Q_INVOKABLE bool attach(QObject *device);
    void attachDevice(QIODevice *device);
    Q_INVOKABLE void detach();
    Q_INVOKABLE void feed(const QByteArray &bytes);
    Q_INVOKABLE void end();

signals:
    void parserChanged();
    void deviceChanged();
    void bufferedBytesChanged();
    void bytesReceived(const QByteArray &bytes);

private:
    void readAvailable();
    void disconnectDevice();

    QPointer<DataStreamParser> m_parser;
    QPointer<QIODevice> m_device;
    QByteArray m_buffer;
    QList<QMetaObject::Connection> m_deviceConnections;
};

} // namespace LfQuick
