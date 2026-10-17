#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTest>

class PlatformQmlSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void platformTypesInstantiateFromQml()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQml
import LfQuick.Platform

QtObject {
    id: root

    property KeyboardWedgeScanner wedge: KeyboardWedgeScanner {
        enabled: true
    }
    property MockScanner mock: MockScanner {}
    property InputRouter router: InputRouter {
        scanner: root.wedge
    }
    property InputTarget directTarget: InputTarget {
        targetId: "direct-target"
        enabled: true
        active: true
        acceptScanner: true
        priority: 5
    }
    property InputSourceRegistration sourceRegistration: InputSourceRegistration {
        router: root.router
        source: root.mock
    }
    property ScannerTarget scannerTarget: ScannerTarget {
        router: root.router
        target: root
        targetId: "scanner-target"
        autoActive: false
        manualActive: true
        acceptScanner: true
        priority: 10
    }
    property HotkeyRouter hotkeys: HotkeyRouter {}
    property SystemAppearance appearance: SystemAppearance {}
    property ScreenService screens: ScreenService {}
    property ClipboardService clipboard: ClipboardService {}
    property FileWatcher watcher: FileWatcher {}
    property RuntimeCapabilities capabilities: RuntimeCapabilities {}
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickPlatformSmoke.qml")));

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
        delete object;
    }
};

QTEST_MAIN(PlatformQmlSmokeTest)
#include "tst_platform_qml.moc"
