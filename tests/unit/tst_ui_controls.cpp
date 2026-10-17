#include <QDebug>
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTemporaryDir>
#include <QTest>

class UiControlsSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void allBaseControlsInstantiate()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

Item {
    width: 1000
    height: 800

    PersistentState {
        key: "smoke-state"
        property int selectedIndex: 2
    }

    LifetimeLease { object: parent }
    QmlObjectFactory { }
    LfLazyLoader { }

    LfButton { text: "Button" }
    LfTextField { text: "Text" }
    LfCheckBox { text: "Check"; checked: true }
    LfRadioButton { text: "Radio"; checked: true }
    LfSwitch { text: "Switch"; checked: true }
    LfComboBox { model: ["A", "B"] }
    LfSlider { from: 0; to: 100; value: 25 }
    LfProgressBar { from: 0; to: 100; value: 25 }
    LfSearchBox { text: "sample" }
    LfFilterInput { text: "ready" }
    LfPasswordField { text: "secret" }
    LfStatusIndicator { status: LfStatusIndicator.Success; text: "Ready" }
    LfNumberInput { from: 0; to: 100; value: 42 }
    LfRangeInput { from: 0; to: 100; first.value: 20; second.value: 80 }
    LfProgressRing { running: true }
    LfDatePicker { value: new Date(2026, 9, 17) }
    LfTimePicker { value: new Date(2026, 9, 17, 10, 30) }
    LfDateTimePicker { value: new Date(2026, 9, 17, 10, 30) }
    LfDateRangePicker {
        startDate: new Date(2026, 9, 1)
        endDate: new Date(2026, 9, 17)
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBaseControls.qml")));

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

    void lazyLoaderCanForceBackgroundLoadAndUnload()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString childPath = temporary.filePath(QStringLiteral("LazyChild.qml"));

        QFile child(childPath);
        QVERIFY(child.open(QIODevice::WriteOnly | QIODevice::Truncate));
        const QByteArray childQml =
            "import QtQml\n"
            "QtObject { property int answer: 42 }\n";
        QCOMPARE(child.write(childQml), qint64(childQml.size()));
        child.close();

        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQml
import LfQuick

QtObject {
    property LfLazyLoader loader: LfLazyLoader {
        objectName: "loader"
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickLazyLoader.qml")));

        QObject *root = component.create();
        QVERIFY2(root, qPrintable(component.errorString()));
        QObject *loader = root->findChild<QObject *>(QStringLiteral("loader"));
        QVERIFY(loader);

        QVERIFY(loader->setProperty("source", QUrl::fromLocalFile(childPath)));
        QVERIFY(loader->setProperty(
            "initialProperties",
            QVariantMap{{QStringLiteral("answer"), 17}}));
        QVERIFY(loader->setProperty("loading", true));

        // Reading item intentionally forces any remaining incubation to finish.
        QObject *loaded = loader->property("item").value<QObject *>();
        QVERIFY2(loaded, qPrintable(loader->property("errorString").toString()));
        QCOMPARE(loaded->property("answer").toInt(), 17);
        QVERIFY(loader->property("active").toBool());

        QVERIFY(loader->setProperty("active", false));
        QCOMPARE(loader->property("item").value<QObject *>(), nullptr);

        delete root;
    }
};

QTEST_MAIN(UiControlsSmokeTest)
#include "tst_ui_controls.moc"
