#include <LfQuick/Io/FileDocument.h>
#include <LfQuick/Io/LocalIpcClient.h>
#include <LfQuick/Io/LocalIpcServer.h>

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

using namespace LfQuick;

class IoTests : public QObject
{
    Q_OBJECT

private slots:
    void fileDocumentProtectsDirtyContentFromExternalReload()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath(QStringLiteral("settings.json"));
        {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("disk-v1"), qint64(7));
        }

        FileDocument document;
        document.setPath(path);
        QVERIFY(document.load());
        QCOMPARE(document.text(), QStringLiteral("disk-v1"));
        QVERIFY(!document.dirty());

        document.setAutoReload(true);
        QVERIFY(document.watch());
        document.setText(QStringLiteral("local-edit"));
        QVERIFY(document.dirty());

        QSignalSpy conflicts(&document, &FileDocument::externalConflictDetected);
        {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("disk-v2"), qint64(7));
            file.flush();
        }

        QTRY_VERIFY_WITH_TIMEOUT(document.externalConflict(), 3000);
        QVERIFY(conflicts.count() >= 1);
        QCOMPARE(document.text(), QStringLiteral("local-edit"));
        QVERIFY(document.dirty());

        // Explicit reload is the user's conflict-resolution decision and may
        // replace dirty memory with the current disk state.
        QVERIFY(document.reload());
        QCOMPARE(document.text(), QStringLiteral("disk-v2"));
        QVERIFY(!document.dirty());
        QVERIFY(!document.externalConflict());
    }

    void localIpcSupportsTypedRpcAndGenericMessages()
    {
        const QString serverName = QStringLiteral("lfquick-io-test-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

        LocalIpcServer server;
        server.removeStaleServer(serverName);
        QVERIFY2(server.listen(serverName), qPrintable(server.errorString()));

        QSignalSpy serverConnected(&server, &LocalIpcServer::clientConnected);
        QSignalSpy serverRequests(&server, &LocalIpcServer::requestReceived);
        QSignalSpy serverMessages(&server, &LocalIpcServer::messageReceived);

        LocalIpcClient client;
        QSignalSpy clientConnected(&client, &LocalIpcClient::connectedToServer);
        QSignalSpy replies(&client, &LocalIpcClient::replyReceived);

        QVERIFY(client.connectToServer(serverName));
        QTRY_COMPARE_WITH_TIMEOUT(clientConnected.count(), 1, 3000);
        QTRY_COMPARE_WITH_TIMEOUT(serverConnected.count(), 1, 3000);
        QVERIFY(client.connected());
        QCOMPARE(server.clientCount(), 1);

        const QString requestId = client.request(
            QStringLiteral("echo"),
            {{QStringLiteral("value"), 42}});
        QVERIFY2(!requestId.isEmpty(), qPrintable(client.errorString()));

        QTRY_COMPARE_WITH_TIMEOUT(serverRequests.count(), 1, 3000);
        const QList<QVariant> requestArguments = serverRequests.takeFirst();
        QCOMPARE(requestArguments.at(1).toString(), requestId);
        QCOMPARE(requestArguments.at(2).toString(), QStringLiteral("echo"));
        QCOMPARE(requestArguments.at(3).toMap().value(QStringLiteral("value")).toInt(), 42);

        const QString clientId = requestArguments.at(0).toString();
        QVERIFY(!clientId.isEmpty());
        QVERIFY(server.reply(
            clientId,
            requestId,
            QVariantMap{{QStringLiteral("accepted"), true}}));

        QTRY_COMPARE_WITH_TIMEOUT(replies.count(), 1, 3000);
        const QList<QVariant> replyArguments = replies.takeFirst();
        QCOMPARE(replyArguments.at(0).toString(), requestId);
        QVERIFY(replyArguments.at(1).toMap().value(QStringLiteral("accepted")).toBool());

        // sendMessage() intentionally remains a generic transport API. Custom
        // typed envelopes that are not one of request/notification/response
        // must continue to flow through messageReceived().
        const int messagesBeforeGeneric = serverMessages.count();
        QVERIFY(client.sendMessage({
            {QStringLiteral("type"), QStringLiteral("telemetry")},
            {QStringLiteral("sequence"), 7}
        }));
        QTRY_COMPARE_WITH_TIMEOUT(serverMessages.count(), messagesBeforeGeneric + 1, 3000);
        QCOMPARE(serverMessages.constLast().at(1).toMap().value(QStringLiteral("sequence")).toInt(), 7);

        // Known RPC types are validated before transmission. A malformed
        // request must not enter the byte stream or the server's typed path.
        const int messagesBeforeInvalid = serverMessages.count();
        QVERIFY(!client.sendMessage({
            {QStringLiteral("type"), QStringLiteral("request")},
            {QStringLiteral("method"), QStringLiteral("missing-id")}
        }));
        QVERIFY(client.errorString().contains(QStringLiteral("id"), Qt::CaseInsensitive));
        QTest::qWait(50);
        QCOMPARE(serverMessages.count(), messagesBeforeInvalid);

        client.abort();
        server.close();
        server.removeStaleServer(serverName);
    }
};

QTEST_MAIN(IoTests)
#include "tst_io.moc"
