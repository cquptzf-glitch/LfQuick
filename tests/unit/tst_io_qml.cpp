#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTest>

class IoQmlSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void ioTypesInstantiateFromQml()
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
};

QTEST_MAIN(IoQmlSmokeTest)
#include "tst_io_qml.moc"
