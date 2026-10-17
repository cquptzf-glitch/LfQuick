#include <LfQuick/Io/LocalIpcClient.h>

#include "IpcFrameCodec_p.h"

#include <QLocalSocket>
#include <QUuid>

#include <algorithm>

namespace LfQuick {
namespace {

QVariantMap requestMessage(const QString &id,
                           const QString &method,
                           const QVariantMap &parameters)
{
    return {
        {QStringLiteral("type"), QStringLiteral("request")},
        {QStringLiteral("id"), id},
        {QStringLiteral("method"), method},
        {QStringLiteral("params"), parameters}
    };
}

} // namespace

LocalIpcClient::LocalIpcClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
{
    // Keep Qt's own socket buffer bounded as a first line of defense. The
    // parser below additionally never lets its application buffer exceed one
    // maximum-sized frame.
    m_socket->setReadBufferSize(IpcFrameCodec::maximumBufferedBytes());

    connect(m_socket, &QLocalSocket::stateChanged, this,
            [this](QLocalSocket::LocalSocketState state) {
        switch (state) {
        case QLocalSocket::UnconnectedState: setState(State::Disconnected); break;
        case QLocalSocket::ConnectingState: setState(State::Connecting); break;
        case QLocalSocket::ConnectedState: setState(State::Connected); break;
        case QLocalSocket::ClosingState: setState(State::Disconnecting); break;
        }
    });
    connect(m_socket, &QLocalSocket::connected,
            this, &LocalIpcClient::connectedToServer);
    connect(m_socket, &QLocalSocket::disconnected, this, [this] {
        m_buffer.clear();
        setState(State::Disconnected);
        emit disconnectedFromServer();
    });
    connect(m_socket, &QLocalSocket::readyRead,
            this, &LocalIpcClient::readAvailable);
    connect(m_socket, &QLocalSocket::errorOccurred, this,
            [this](QLocalSocket::LocalSocketError) {
        setErrorString(m_socket->errorString());
    });
}

LocalIpcClient::~LocalIpcClient()
{
    if (m_socket)
        m_socket->abort();
}

bool LocalIpcClient::connectToServer(const QString &serverName)
{
    const QString normalized = serverName.trimmed();
    if (normalized.isEmpty()) {
        setErrorString(QStringLiteral("Local IPC server name is empty"));
        return false;
    }

    if (m_serverName != normalized) {
        m_serverName = normalized;
        emit serverNameChanged();
    }

    m_buffer.clear();
    setErrorString({});
    if (m_socket->state() != QLocalSocket::UnconnectedState)
        m_socket->abort();
    m_socket->connectToServer(m_serverName);
    setState(State::Connecting);
    return true;
}

void LocalIpcClient::disconnectFromServer()
{
    if (m_socket && m_socket->state() != QLocalSocket::UnconnectedState)
        m_socket->disconnectFromServer();
}

void LocalIpcClient::abort()
{
    if (m_socket)
        m_socket->abort();
    m_buffer.clear();
    setState(State::Disconnected);
}

bool LocalIpcClient::sendMessage(const QVariantMap &message)
{
    return writeFrame(message);
}

QString LocalIpcClient::request(const QString &method,
                                const QVariantMap &parameters)
{
    if (method.trimmed().isEmpty()) {
        setErrorString(QStringLiteral("IPC request method is empty"));
        return {};
    }
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return writeFrame(requestMessage(id, method, parameters)) ? id : QString{};
}

bool LocalIpcClient::notify(const QString &method,
                            const QVariantMap &parameters)
{
    if (method.trimmed().isEmpty()) {
        setErrorString(QStringLiteral("IPC notification method is empty"));
        return false;
    }
    return writeFrame({
        {QStringLiteral("type"), QStringLiteral("notification")},
        {QStringLiteral("method"), method},
        {QStringLiteral("params"), parameters}
    });
}

bool LocalIpcClient::reply(const QString &requestId, const QVariant &result)
{
    if (requestId.trimmed().isEmpty())
        return false;
    return writeFrame({
        {QStringLiteral("type"), QStringLiteral("response")},
        {QStringLiteral("id"), requestId},
        {QStringLiteral("result"), result}
    });
}

bool LocalIpcClient::replyError(const QString &requestId,
                                const QString &code,
                                const QString &message,
                                const QVariantMap &details)
{
    if (requestId.trimmed().isEmpty())
        return false;
    return writeFrame({
        {QStringLiteral("type"), QStringLiteral("response")},
        {QStringLiteral("id"), requestId},
        {QStringLiteral("error"), QVariantMap{
            {QStringLiteral("code"), code},
            {QStringLiteral("message"), message},
            {QStringLiteral("details"), details}
        }}
    });
}

void LocalIpcClient::readAvailable()
{
    if (!m_socket)
        return;

    const qsizetype maxBuffered = IpcFrameCodec::maximumBufferedBytes();
    const qsizetype chunkSize = IpcFrameCodec::recommendedReadChunkBytes();

    while (m_socket && m_socket->bytesAvailable() > 0) {
        const qsizetype room = maxBuffered - m_buffer.size();
        if (room <= 0) {
            setErrorString(QStringLiteral("IPC receive buffer exceeded the frame limit"));
            m_socket->abort();
            m_buffer.clear();
            return;
        }

        const qint64 wanted = static_cast<qint64>(std::min<qsizetype>(
            {room, chunkSize, static_cast<qsizetype>(m_socket->bytesAvailable())}));
        const QByteArray bytes = m_socket->read(wanted);
        if (bytes.isEmpty())
            break;
        m_buffer.append(bytes);

        while (true) {
            QVariantMap message;
            QString error;
            const auto result = IpcFrameCodec::takeFrame(m_buffer, message, &error);
            if (result == IpcFrameCodec::DecodeResult::Incomplete)
                break;
            if (result == IpcFrameCodec::DecodeResult::Error) {
                setErrorString(error);
                m_socket->abort();
                m_buffer.clear();
                return;
            }
            if (!IpcFrameCodec::validateTypedEnvelope(message, &error)) {
                setErrorString(error);
                m_socket->abort();
                m_buffer.clear();
                return;
            }
            handleMessage(message);
        }
    }
}

void LocalIpcClient::handleMessage(const QVariantMap &message)
{
    emit messageReceived(message);

    const QString type = message.value(QStringLiteral("type")).toString();
    const QString id = message.value(QStringLiteral("id")).toString();
    if (type == QStringLiteral("request")) {
        emit requestReceived(id,
                             message.value(QStringLiteral("method")).toString(),
                             message.value(QStringLiteral("params")).toMap());
    } else if (type == QStringLiteral("notification")) {
        emit notificationReceived(message.value(QStringLiteral("method")).toString(),
                                  message.value(QStringLiteral("params")).toMap());
    } else if (type == QStringLiteral("response")) {
        const QVariantMap error = message.value(QStringLiteral("error")).toMap();
        if (!error.isEmpty()) {
            emit requestFailed(id,
                               error.value(QStringLiteral("code")).toString(),
                               error.value(QStringLiteral("message")).toString(),
                               error.value(QStringLiteral("details")).toMap());
        } else {
            emit replyReceived(id, message.value(QStringLiteral("result")));
        }
    }
}

bool LocalIpcClient::writeFrame(const QVariantMap &message)
{
    if (!m_socket || m_socket->state() != QLocalSocket::ConnectedState) {
        setErrorString(QStringLiteral("IPC client is not connected"));
        return false;
    }

    QString error;
    if (!IpcFrameCodec::validateTypedEnvelope(message, &error)) {
        setErrorString(error);
        return false;
    }

    const QByteArray frame = IpcFrameCodec::encode(message, &error);
    if (frame.isEmpty() && !error.isEmpty()) {
        setErrorString(error);
        return false;
    }
    if (!IpcFrameCodec::writeAll(m_socket, frame, &error)) {
        setErrorString(error.isEmpty() ? m_socket->errorString() : error);
        return false;
    }
    m_socket->flush();
    setErrorString({});
    return true;
}

void LocalIpcClient::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged();
}

void LocalIpcClient::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

} // namespace LfQuick
