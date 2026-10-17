#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTest>

class ModelsQmlSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void modelsTypesInstantiateFromQml()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQml
import LfQuick.Models

QtObject {
    id: root

    property LfObjectListModel objects: LfObjectListModel {}
    property LfScriptModel script: LfScriptModel {
        keyProperty: "id"
        values: [
            ({ id: "a", value: 1 }),
            ({ id: "b", value: 2 })
        ]
    }
    property LfObjectRepeater repeater: LfObjectRepeater {
        keyProperty: "id"
        model: [
            ({ id: "a", value: 1 }),
            ({ id: "b", value: 2 })
        ]
        delegate: Component {
            QtObject {
                required property var modelData
                required property int modelIndex
                property string identity: modelData.id
            }
        }
    }

    property int objectCount: objects.count
    property int scriptCount: script.count
    property int repeatedCount: repeater.count
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickModelsSmoke.qml")));

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
        QCOMPARE(object->property("objectCount").toInt(), 0);
        QCOMPARE(object->property("scriptCount").toInt(), 2);
        QCOMPARE(object->property("repeatedCount").toInt(), 2);
        delete object;
    }
};

QTEST_MAIN(ModelsQmlSmokeTest)
#include "tst_models_qml.moc"
