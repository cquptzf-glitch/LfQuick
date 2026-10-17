#include <LfQuick/Io/LocalIpcServer.h>

#include "IpcFrameCodec_p.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QUuid>

#include <algorithm>
#include <utility>

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

QVariantMap notificationMessage(const QString &method,
                                const QVariantMap &parameters)
{
    return {
        {QStringLiteral("type"), QStringLiteral("notification")},
        {QStringLiteral("method"), method},
        {QStringLiteral("params"), parameters}
    };
}

} // namespace

LocalIpcServer::LocalIpcServer(QObject *parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection,
            this, &LocalIpcServer::acceptPendingConnections);
    connect(m_server, &QLocalServer::acceptError, this,
            [this](QAbstractSocket::SocketError) {
        setErrorString(m_server->errorString());
    });
}

LocalIpcServer::~LocalIpcServer()
{
    close();
}

bool LocalIpcServer::listening() const
{
    return m_server && m_server->isListening();
}

QStringList LocalIpcServer::clientIds() const
{
    QStringList ids = m_clients.keys();
    ids.sort();
    return ids;
}

bool LocalIpcServer::listen(const QString &name)
{
    const QString normalized = name.trimmed();
    if (normalized.isEmpty()) {
        setErrorString(QStringLiteral("Local IPC server name is empty"));
        return false;
    }

    close();
    m_name = normalized;
    setErrorString({});
    if (!m_server->listen(m_name)) {
        setErrorString(m_server->errorString());
        emit listeningChanged();
        return false;
    }

    emit listeningChanged();
    return true;
}

void LocalIpcServer::close()
{
    const bool wasListening = listening();
    const QStringList ids = m_clients.keys();

    for (const QString &id : ids) {
        auto it = m_clients.find(id);
        if (it == m_clients.end())
            continue;
        if (it->socket) {
            QObject::disconnect(it->socket, nullptr, this, nullptr);
            it->socket->abort();
            it->socket->deleteLater();
        }
        emit clientDisconnected(id);
    }
    if (!m_clients.isEmpty()) {
        m_clients.clear();
        emit clientsChanged();
    }

    if (m_server && m_server->isListening())
        m_server->close();
    if (wasListening)
        emit listeningChanged();
}

bool LocalIpcServer::removeStaleServer(const QString &name)
{
    if (name.trimmed().isEmpty())
        return false;
    return QLocalServer::removeServer(name.trimmed());
}

bool LocalIpcServer::sendMessage(const QString &clientId, const QVariantMap &message)
{
    return writeFrame(clientId, message);
}

int LocalIpcServer::broadcast(const QVariantMap &message)
{
    int sent = 0;
    const QStringList ids = m_clients.keys();
    for (const QString &id : ids) {
        if (writeFrame(id, message))
            ++sent;
    }
    return sent;
}

QString LocalIpcServer::request(const QString &clientId,
                                const QString &method,
                                const QVariantMap &parameters)
{
    if (method.trimmed().isEmpty()) {
        setErrorString(QStringLiteral("IPC request method is empty"));
        return {};
    }
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return writeFrame(clientId, requestMessage(id, method, parameters)) ? id : QString{};
}

bool LocalIpcServer::notify(const QString &clientId,
                            const QString &method,
                            const QVariantMap &parameters)
{
    if (method.trimmed().isEmpty()) {
        setErrorString(QStringLiteral("IPC notification method is empty"));
        return false;
    }
    return writeFrame(clientId, notificationMessage(method, parameters));
}

bool LocalIpcServer::reply(const QString &clientId,
                           const QString &requestId,
                           const QVariant &result)
{
    if (requestId.trimmed().isEmpty())
        return false;
    return writeFrame(clientId, {
        {QStringLiteral("type"), QStringLiteral("response")},
        {QStringLiteral("id"), requestId},
        {QStringLiteral("result"), result}
    });
}

bool LocalIpcServer::replyError(const QString &clientId,
                                const QString &requestId,
                                const QString &code,
                                const QString &message,
                                const QVariantMap &details)
{
    if (requestId.trimmed().isEmpty())
        return false;
    return writeFrame(clientId, {
        {QStringLiteral("type"), QStringLiteral("response")},
        {QStringLiteral("id"), requestId},
        {QStringLiteral("error"), QVariantMap{
            {QStringLiteral("code"), code},
            {QStringLiteral("message"), message},
            {QStringLiteral("details"), details}
        }}
    });
}

void LocalIpcServer::acceptPendingConnections()
{
    if (!m_server)
        return;

    bool changed = false;
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (!socket)
            continue;

        socket->setReadBufferSize(IpcFrameCodec::maximumBufferedBytes());

        const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        ClientState state;
        state.socket = socket;
        m_clients.insert(id, state);
        changed = true;

        connect(socket, &QLocalSocket::readyRead, this,
                [this, id] { readClient(id); });
        connect(socket, &QLocalSocket::disconnected, this,
                [this, id] { removeClient(id); });
        connect(socket, &QLocalSocket::errorOccurred, this,
                [this, socket](QLocalSocket::LocalSocketError) {
            if (socket)
                setErrorString(socket->errorString());
        });
        connect(socket, &QObject::destroyed, this,
                [this, id] { removeClient(id); });
        emit clientConnected(id);
    }

    if (changed)
        emit clientsChanged();
}

void LocalIpcServer::readClient(const QString &clientId)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end() || !it->socket)
        return;

    const qsizetype maxBuffered = IpcFrameCodec::maximumBufferedBytes();
    const qsizetype chunkSize = IpcFrameCodec::recommendedReadChunkBytes();

    while (true) {
        it = m_clients.find(clientId);
        if (it == m_clients.end() || !it->socket)
            return;
        if (it->socket->bytesAvailable() <= 0)
            return;

        const qsizetype room = maxBuffered - it->buffer.size();
        if (room <= 0) {
            setErrorString(QStringLiteral("IPC receive buffer exceeded the frame limit"));
            it->socket->abort();
            return;
        }

        const qint64 wanted = static_cast<qint64>(std::min<qsizetype>(
            {room, chunkSize, static_cast<qsizetype>(it->socket->bytesAvailable())}));
        const QByteArray bytes = it->socket->read(wanted);
        if (bytes.isEmpty())
            return;
        it->buffer.append(bytes);

        while (true) {
            QVariantMap message;
            QString error;
            const auto result = IpcFrameCodec::takeFrame(it->buffer, message, &error);
            if (result == IpcFrameCodec::DecodeResult::Incomplete)
                break;
            if (result == IpcFrameCodec::DecodeResult::Error) {
                setErrorString(error);
                if (it->socket)
                    it->socket->abort();
                return;
            }
            if (!IpcFrameCodec::validateTypedEnvelope(message, &error)) {
                setErrorString(error);
                if (it->socket)
                    it->socket->abort();
                return;
            }

            handleMessage(clientId, message);
            it = m_clients.find(clientId);
            if (it == m_clients.end() || !it->socket)
                return;
        }
    }
}

void LocalIpcServer::removeClient(const QString &clientId)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end())
        return;

    QPointer<QLocalSocket> socket = it->socket;
    m_clients.erase(it);
    if (socket) {
        QObject::disconnect(socket, nullptr, this, nullptr);
        socket->deleteLater();
    }
    emit clientDisconnected(clientId);
    emit clientsChanged();
}

void LocalIpcServer::handleMessage(const QString &clientId,
                                   const QVariantMap &message)
{
    emit messageReceived(clientId, message);

    const QString type = message.value(QStringLiteral("type")).toString();
    const QString id = message.value(QStringLiteral("id")).toString();
    if (type == QStringLiteral("request")) {
        emit requestReceived(clientId,
                             id,
                             message.value(QStringLiteral("method")).toString(),
                             message.value(QStringLiteral("params")).toMap());
    } else if (type == QStringLiteral("notification")) {
        emit notificationReceived(clientId,
                                  message.value(QStringLiteral("method")).toString(),
                                  message.value(QStringLiteral("params")).toMap());
    } else if (type == QStringLiteral("response")) {
        const QVariantMap error = message.value(QStringLiteral("error")).toMap();
        if (!error.isEmpty()) {
            emit requestFailed(clientId,
                               id,
                               error.value(QStringLiteral("code")).toString(),
                               error.value(QStringLiteral("message")).toString(),
                               error.value(QStringLiteral("details")).toMap());
        } else {
            emit replyReceived(clientId, id, message.value(QStringLiteral("result")));
        }
    }
}

bool LocalIpcServer::writeFrame(const QString &clientId,
                                const QVariantMap &message)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end() || !it->socket
        || it->socket->state() != QLocalSocket::ConnectedState) {
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

    if (!IpcFrameCodec::writeAll(it->socket, frame, &error)) {
        setErrorString(error.isEmpty() ? it->socket->errorString() : error);
        return false;
    }
    it->socket->flush();
    setErrorString({});
    return true;
}

void LocalIpcServer::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

} // namespace LfQuick
