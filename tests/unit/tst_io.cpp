#include <LfQuick/Io/DataStream.h>
#include <LfQuick/Io/FileDocument.h>
#include <LfQuick/Io/JsonDocument.h>
#include <LfQuick/Io/LocalIpcClient.h>
#include <LfQuick/Io/LocalIpcServer.h>
#include <LfQuick/Io/ProcessRunner.h>

#include <QDebug>
#include <QFile>
#include <QList>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

using namespace LfQuick;

class IoTests : public QObject
{
    Q_OBJECT

private slots:
    void qmlModuleTypesInstantiate()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQml
import LfQuick.Io

QtObject {
    id: root

    property LfFileWatcher watcher: LfFileWatcher {}
    property LfFileDocument fileDocument: LfFileDocument {}
    property LfJsonDocument jsonDocument: LfJsonDocument {}
    property LfDelimitedStreamParser delimiterParser: LfDelimitedStreamParser {}
    property LfStreamCollector collector: LfStreamCollector {
        maxBytes: 1024
    }
    property LfDataStream stream: LfDataStream {
        parser: root.delimiterParser
    }
    property LfProcess process: LfProcess {
        stdoutParser: root.collector
    }
    property LfLocalIpcClient client: LfLocalIpcClient {}
    property LfLocalIpcServer server: LfLocalIpcServer {}

    property bool defaultsValid: !fileDocument.dirty
                                 && !jsonDocument.dirty
                                 && collector.maxBytes === 1024
                                 && !stream.attached
                                 && !process.running
                                 && !client.connected
                                 && !server.listening
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickIoSmoke.qml")));

        if (component.isError()) {
            for (const QQmlError &error : component.errors())
                qWarning().noquote() << error.toString();
        }

        QObject *object = component.create();
        if (!object) {
            for (const QQmlError &error : component.errors())
                qWarning().noquote() << error.toString();
        }
        QVERIFY2(object, qPrintable(component.errorString()));
        QVERIFY(object->property("defaultsValid").toBool());
        delete object;
    }

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

        // A failed Save As is transactional: path/watch/dirty/conflict remain
        // attached to the original document until the target is committed.
        const QString originalPath = document.path();
        const QString blocker = directory.filePath(QStringLiteral("not-a-directory"));
        {
            QFile file(blocker);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("x"), qint64(1));
        }
        QSignalSpy pathChanged(&document, &FileDocument::pathChanged);
        QVERIFY(!document.saveAs(blocker + QStringLiteral("/copy.json")));
        QCOMPARE(document.path(), originalPath);
        QCOMPARE(pathChanged.count(), 0);
        QVERIFY(document.watch());
        QVERIFY(document.dirty());
        QVERIFY(document.externalConflict());
        QVERIFY(!document.errorString().isEmpty());

        // Explicit reload is the user's conflict-resolution decision and may
        // replace dirty memory with the current disk state.
        QVERIFY(document.reload());
        QCOMPARE(document.text(), QStringLiteral("disk-v2"));
        QVERIFY(!document.dirty());
        QVERIFY(!document.externalConflict());
    }

    void jsonDocumentSaveAsFailurePreservesAssociation()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath(QStringLiteral("document.json"));
        {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            const QByteArray initial = QByteArrayLiteral("{\"value\":1}");
            QCOMPARE(file.write(initial), qint64(initial.size()));
        }

        JsonDocument document;
        document.setPath(path);
        QVERIFY2(document.load(), qPrintable(document.errorString()));
        document.setValue(QVariantMap{{QStringLiteral("value"), 2}});
        QVERIFY(document.dirty());

        const QString originalPath = document.path();
        const QString blocker = directory.filePath(QStringLiteral("json-not-a-directory"));
        {
            QFile file(blocker);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("x"), qint64(1));
        }

        QSignalSpy pathChanged(&document, &JsonDocument::pathChanged);
        QVERIFY(!document.saveAs(blocker + QStringLiteral("/copy.json")));
        QCOMPARE(document.path(), originalPath);
        QCOMPARE(pathChanged.count(), 0);
        QVERIFY(document.dirty());
        QCOMPARE(document.value().toMap().value(QStringLiteral("value")).toInt(), 2);
        QVERIFY(!document.errorString().isEmpty());
    }

    void dataStreamTracksParserDestructionAndClearsPartialBuffer()
    {
        DataStream stream;
        auto *parser = new DelimitedStreamParser;
        stream.setParser(parser);
        stream.feed(QByteArrayLiteral("partial"));
        QCOMPARE(stream.bufferedBytes(), qint64(7));

        QSignalSpy parserChanged(&stream, &DataStream::parserChanged);
        QSignalSpy bufferedChanged(&stream, &DataStream::bufferedBytesChanged);
        delete parser;

        QCOMPARE(stream.parser(), nullptr);
        QCOMPARE(stream.bufferedBytes(), qint64(0));
        QCOMPARE(parserChanged.count(), 1);
        QCOMPARE(bufferedChanged.count(), 1);
    }

    void delimitedParserDropsOversizedRecordAndRecovers()
    {
        DataStream stream;
        DelimitedStreamParser parser;
        parser.setMaxBufferedBytes(8);
        stream.setParser(&parser);

        QSignalSpy overflow(&parser, &DelimitedStreamParser::bufferOverflow);
        QSignalSpy reads(&parser, &DataStreamParser::read);

        // No delimiter: once the record exceeds the configured limit, the
        // partial record is discarded rather than retained without bound.
        stream.feed(QByteArrayLiteral("0123456789"));
        QCOMPARE(stream.bufferedBytes(), qint64(0));
        QCOMPARE(reads.count(), 0);
        QVERIFY(overflow.count() >= 1);

        // While discarding an oversized record, further chunks remain bounded
        // and are ignored until the next delimiter establishes a clean record
        // boundary.
        stream.feed(QByteArrayLiteral("abcdef"));
        QCOMPARE(stream.bufferedBytes(), qint64(0));
        QCOMPARE(reads.count(), 0);

        stream.feed(QByteArrayLiteral("\nok\n"));
        QCOMPARE(stream.bufferedBytes(), qint64(0));
        QCOMPARE(reads.count(), 1);
        QCOMPARE(reads.at(0).at(0).toString(), QStringLiteral("ok"));

        // Ending during discard mode must not emit an oversized partial suffix
        // as if it were a valid record.
        stream.feed(QByteArrayLiteral("123456789"));
        const int readsBeforeEnd = reads.count();
        stream.end();
        QCOMPARE(reads.count(), readsBeforeEnd);
        QCOMPARE(stream.bufferedBytes(), qint64(0));
    }

    void delimitedParserRecoversAcrossSplitMultiByteDelimiter()
    {
        DataStream stream;
        DelimitedStreamParser parser;
        parser.setDelimiter(QByteArrayLiteral("\r\n"));
        parser.setMaxBufferedBytes(8);
        stream.setParser(&parser);

        QSignalSpy reads(&parser, &DataStreamParser::read);
        QSignalSpy overflow(&parser, &DelimitedStreamParser::bufferOverflow);

        // The oversized record ends with the first byte of the delimiter. The
        // parser must retain only that delimiter prefix rather than the record.
        stream.feed(QByteArrayLiteral("0123456789\r"));
        QCOMPARE(stream.bufferedBytes(), qint64(1));
        QCOMPARE(reads.count(), 0);
        QVERIFY(overflow.count() >= 1);

        // The next chunk completes the split delimiter and immediately starts
        // a valid record. Recovery must emit only the valid post-boundary row.
        stream.feed(QByteArrayLiteral("\nok\r\n"));
        QCOMPARE(stream.bufferedBytes(), qint64(0));
        QCOMPARE(reads.count(), 1);
        QCOMPARE(reads.at(0).at(0).toString(), QStringLiteral("ok"));
    }

    void processFailedStartCompletesExactlyOnce()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        ProcessRunner runner;
        StreamCollector collector;
        runner.setStdoutParser(&collector);
        runner.setProgram(temporary.filePath(QStringLiteral("definitely-missing-executable")));

        QSignalSpy finished(&runner, &ProcessRunner::finished);
        QSignalSpy parserFinished(&collector, &DataStreamParser::finished);

        QVERIFY(runner.start());
        QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 3000);
        QCOMPARE(parserFinished.count(), 1);
        QCOMPARE(runner.state(), ProcessRunner::State::NotRunning);
        QVERIFY(!runner.running());
        QCOMPARE(runner.exitCode(), -1);
        QCOMPARE(runner.exitStatus(), ProcessRunner::ExitStatus::Unknown);
        QVERIFY(!runner.errorString().isEmpty());

        // No delayed QProcess signal may complete the same logical run twice.
        QTest::qWait(50);
        QCOMPARE(finished.count(), 1);
        QCOMPARE(parserFinished.count(), 1);
    }

    void processTracksParserDestruction()
    {
        ProcessRunner runner;

        auto *stdoutParser = new DelimitedStreamParser;
        runner.setStdoutParser(stdoutParser);
        QCOMPARE(runner.stdoutParser(), stdoutParser);
        QSignalSpy stdoutChanged(&runner, &ProcessRunner::stdoutParserChanged);
        delete stdoutParser;
        QCOMPARE(runner.stdoutParser(), nullptr);
        QCOMPARE(stdoutChanged.count(), 1);

        auto *stderrParser = new DelimitedStreamParser;
        runner.setStderrParser(stderrParser);
        QCOMPARE(runner.stderrParser(), stderrParser);
        QSignalSpy stderrChanged(&runner, &ProcessRunner::stderrParserChanged);
        delete stderrParser;
        QCOMPARE(runner.stderrParser(), nullptr);
        QCOMPARE(stderrChanged.count(), 1);
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

        QList<LocalIpcClient::State> disconnectStates;
        connect(&client, &LocalIpcClient::stateChanged, &client, [&] {
            disconnectStates.push_back(client.state());
        });
        QSignalSpy disconnected(&client, &LocalIpcClient::disconnectedFromServer);
        client.disconnectFromServer();
        QTRY_COMPARE_WITH_TIMEOUT(disconnected.count(), 1, 3000);
        QCOMPARE(client.state(), LocalIpcClient::State::Disconnected);
        QVERIFY(!client.connected());
        QVERIFY(!disconnectStates.contains(LocalIpcClient::State::Connecting));
        QVERIFY(disconnectStates.contains(LocalIpcClient::State::Disconnecting)
                || disconnectStates == QList<LocalIpcClient::State>{
                       LocalIpcClient::State::Disconnected});

        server.close();
        server.removeStaleServer(serverName);
    }
};

QTEST_MAIN(IoTests)
#include "tst_io.moc"
