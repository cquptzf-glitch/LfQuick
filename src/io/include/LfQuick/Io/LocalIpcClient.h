#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

class QLocalSocket;

namespace LfQuick {

class LFQUICKIO_EXPORT LocalIpcClient final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfLocalIpcClient)
    Q_PROPERTY(QString serverName READ serverName NOTIFY serverNameChanged FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    enum class State {
        Disconnected,
        Connecting,
        Connected
    };
    Q_ENUM(State)

    explicit LocalIpcClient(QObject *parent = nullptr);
    ~LocalIpcClient() override;

    QString serverName() const { return m_serverName; }
    State state() const noexcept { return m_state; }
    bool connected() const noexcept { return m_state == State::Connected; }
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE bool connectToServer(const QString &serverName);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void abort();

    Q_INVOKABLE bool sendMessage(const QVariantMap &message);
    Q_INVOKABLE QString request(const QString &method,
                                const QVariantMap &parameters = {});
    Q_INVOKABLE bool notify(const QString &method,
                            const QVariantMap &parameters = {});
    Q_INVOKABLE bool reply(const QString &requestId,
                           const QVariant &result = {});
    Q_INVOKABLE bool replyError(const QString &requestId,
                                const QString &code,
                                const QString &message,
                                const QVariantMap &details = {});

signals:
    void serverNameChanged();
    void stateChanged();
    void errorStringChanged();
    void connectedToServer();
    void disconnectedFromServer();
    void messageReceived(const QVariantMap &message);
    void requestReceived(const QString &requestId,
                         const QString &method,
                         const QVariantMap &parameters);
    void notificationReceived(const QString &method,
                              const QVariantMap &parameters);
    void replyReceived(const QString &requestId, const QVariant &result);
    void requestFailed(const QString &requestId,
                       const QString &code,
                       const QString &message,
                       const QVariantMap &details);

private:
    void readAvailable();
    void handleMessage(const QVariantMap &message);
    bool writeFrame(const QVariantMap &message);
    void setState(State state);
    void setErrorString(const QString &error);

    QLocalSocket *m_socket = nullptr;
    QString m_serverName;
    State m_state = State::Disconnected;
    QString m_errorString;
    QByteArray m_buffer;
};

} // namespace LfQuick
