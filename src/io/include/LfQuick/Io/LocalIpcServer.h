#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QLocalSocket>
#include <QtQmlIntegration/qqmlintegration.h>

class QLocalServer;

namespace LfQuick {

class LFQUICKIO_EXPORT LocalIpcServer final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfLocalIpcServer)
    Q_PROPERTY(QString name READ name NOTIFY listeningChanged FINAL)
    Q_PROPERTY(bool listening READ listening NOTIFY listeningChanged FINAL)
    Q_PROPERTY(int clientCount READ clientCount NOTIFY clientsChanged FINAL)
    Q_PROPERTY(QStringList clientIds READ clientIds NOTIFY clientsChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    explicit LocalIpcServer(QObject *parent = nullptr);
    ~LocalIpcServer() override;

    QString name() const { return m_name; }
    bool listening() const;
    int clientCount() const noexcept { return m_clients.size(); }
    QStringList clientIds() const;
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE bool listen(const QString &name);
    Q_INVOKABLE void close();
    Q_INVOKABLE bool removeStaleServer(const QString &name);

    Q_INVOKABLE bool sendMessage(const QString &clientId, const QVariantMap &message);
    Q_INVOKABLE int broadcast(const QVariantMap &message);
    Q_INVOKABLE QString request(const QString &clientId,
                                const QString &method,
                                const QVariantMap &parameters = {});
    Q_INVOKABLE bool notify(const QString &clientId,
                            const QString &method,
                            const QVariantMap &parameters = {});
    Q_INVOKABLE bool reply(const QString &clientId,
                           const QString &requestId,
                           const QVariant &result = {});
    Q_INVOKABLE bool replyError(const QString &clientId,
                                const QString &requestId,
                                const QString &code,
                                const QString &message,
                                const QVariantMap &details = {});

signals:
    void listeningChanged();
    void clientsChanged();
    void errorStringChanged();
    void clientConnected(const QString &clientId);
    void clientDisconnected(const QString &clientId);
    void messageReceived(const QString &clientId, const QVariantMap &message);
    void requestReceived(const QString &clientId,
                         const QString &requestId,
                         const QString &method,
                         const QVariantMap &parameters);
    void notificationReceived(const QString &clientId,
                              const QString &method,
                              const QVariantMap &parameters);
    void replyReceived(const QString &clientId,
                       const QString &requestId,
                       const QVariant &result);
    void requestFailed(const QString &clientId,
                       const QString &requestId,
                       const QString &code,
                       const QString &message,
                       const QVariantMap &details);

private:
    struct ClientState {
        QPointer<QLocalSocket> socket;
        QByteArray buffer;
    };

    void acceptPendingConnections();
    void readClient(const QString &clientId);
    void removeClient(const QString &clientId);
    void handleMessage(const QString &clientId, const QVariantMap &message);
    bool writeFrame(const QString &clientId, const QVariantMap &message);
    void setErrorString(const QString &error);

    QLocalServer *m_server = nullptr;
    QString m_name;
    QString m_errorString;
    QHash<QString, ClientState> m_clients;
};

} // namespace LfQuick
