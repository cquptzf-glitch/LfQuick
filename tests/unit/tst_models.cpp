#include <LfQuick/Models/ObjectListModel.h>
#include <LfQuick/Models/ObjectRepeater.h>
#include <LfQuick/Models/ScriptModel.h>

#include <QAbstractItemModel>
#include <QDebug>
#include <QJSEngine>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QTest>

#include <utility>

using namespace LfQuick;

namespace {

QVariantMap row(QString id, QString label)
{
    return {
        {QStringLiteral("id"), std::move(id)},
        {QStringLiteral("label"), std::move(label)}
    };
}

QJSValue jsRow(QJSEngine &engine, QString id, int value)
{
    return engine.toScriptValue(QVariantMap{
        {QStringLiteral("id"), std::move(id)},
        {QStringLiteral("value"), value}
    });
}

} // namespace

class ModelsTests : public QObject
{
    Q_OBJECT

private slots:
    void qmlModuleTypesInstantiate()
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

    void objectRepeaterSatisfiesRequiredModelPropertiesAndPreservesIdentity()
    {
        QQmlEngine engine;
        QQmlComponent delegate(&engine);
        delegate.setData(R"(
            import QtQml
            QtObject {
                required property var modelData
                required property int modelIndex
                property string identityKey: modelData.id
                property string label: modelData.label
            }
        )", QUrl(QStringLiteral("inmemory:/RequiredDelegate.qml")));
        QVERIFY2(delegate.isReady(), qPrintable(delegate.errorString()));

        ObjectRepeater repeater;
        repeater.setDelegate(&delegate);
        repeater.setKeyProperty(QStringLiteral("id"));
        repeater.setModel(QVariantList{
            row(QStringLiteral("a"), QStringLiteral("A")),
            row(QStringLiteral("b"), QStringLiteral("B"))
        });

        QCOMPARE(repeater.count(), 2);
        QObject *a = repeater.instanceAt(0);
        QObject *b = repeater.instanceAt(1);
        QVERIFY(a);
        QVERIFY(b);
        QCOMPARE(a->property("modelIndex").toInt(), 0);
        QCOMPARE(b->property("modelIndex").toInt(), 1);
        QCOMPARE(a->property("identityKey").toString(), QStringLiteral("a"));
        QCOMPARE(b->property("identityKey").toString(), QStringLiteral("b"));

        // Unique keys preserve object identity even when the model is reordered.
        repeater.setModel(QVariantList{
            row(QStringLiteral("b"), QStringLiteral("B2")),
            row(QStringLiteral("a"), QStringLiteral("A2"))
        });

        QCOMPARE(repeater.count(), 2);
        QCOMPARE(repeater.instanceAt(0), b);
        QCOMPARE(repeater.instanceAt(1), a);
        QCOMPARE(b->property("modelIndex").toInt(), 0);
        QCOMPARE(a->property("modelIndex").toInt(), 1);
        QCOMPARE(b->property("label").toString(), QStringLiteral("B2"));
        QCOMPARE(a->property("label").toString(), QStringLiteral("A2"));
    }

    void objectRepeaterDuplicateKeysFallBackWithoutDroppingRows()
    {
        QQmlEngine engine;
        QQmlComponent delegate(&engine);
        delegate.setData(R"(
            import QtQml
            QtObject {
                required property var modelData
                required property int modelIndex
                property string label: modelData.label
            }
        )", QUrl(QStringLiteral("inmemory:/DuplicateDelegate.qml")));
        QVERIFY2(delegate.isReady(), qPrintable(delegate.errorString()));

        ObjectRepeater repeater;
        repeater.setDelegate(&delegate);
        repeater.setKeyProperty(QStringLiteral("id"));
        repeater.setModel(QVariantList{
            row(QStringLiteral("same"), QStringLiteral("first")),
            row(QStringLiteral("same"), QStringLiteral("second")),
            row(QStringLiteral("other"), QStringLiteral("third"))
        });

        QCOMPARE(repeater.count(), 3);
        QVERIFY(repeater.lastError().contains(QStringLiteral("duplicate"), Qt::CaseInsensitive));
        QObject *first = repeater.instanceAt(0);
        QObject *second = repeater.instanceAt(1);
        QObject *third = repeater.instanceAt(2);
        QVERIFY(first && second && third);

        repeater.setModel(QVariantList{
            row(QStringLiteral("same"), QStringLiteral("first-2")),
            row(QStringLiteral("same"), QStringLiteral("second-2")),
            row(QStringLiteral("other"), QStringLiteral("third-2"))
        });

        QCOMPARE(repeater.count(), 3);
        QCOMPARE(repeater.instanceAt(0), first);
        QCOMPARE(repeater.instanceAt(1), second);
        QCOMPARE(repeater.instanceAt(2), third);
        QCOMPARE(first->property("label").toString(), QStringLiteral("first-2"));
        QCOMPARE(second->property("label").toString(), QStringLiteral("second-2"));
        QCOMPARE(third->property("label").toString(), QStringLiteral("third-2"));
    }

    void objectRepeaterUpdatesUnqualifiedContextBindingsOnReuse()
    {
        QQmlEngine engine;
        QQmlComponent delegate(&engine);
        delegate.setData(R"(
            import QtQml
            QtObject {
                property string labelFromContext: modelData.label
                property int indexFromContext: modelIndex
            }
        )", QUrl(QStringLiteral("inmemory:/ContextDelegate.qml")));
        QVERIFY2(delegate.isReady(), qPrintable(delegate.errorString()));

        ObjectRepeater repeater;
        repeater.setDelegate(&delegate);
        repeater.setKeyProperty(QStringLiteral("id"));
        repeater.setModel(QVariantList{
            row(QStringLiteral("a"), QStringLiteral("before"))
        });

        QCOMPARE(repeater.count(), 1);
        QObject *instance = repeater.instanceAt(0);
        QVERIFY(instance);
        QCOMPARE(instance->property("labelFromContext").toString(), QStringLiteral("before"));
        QCOMPARE(instance->property("indexFromContext").toInt(), 0);

        repeater.setModel(QVariantList{
            row(QStringLiteral("a"), QStringLiteral("after"))
        });

        QCOMPARE(repeater.instanceAt(0), instance);
        QCOMPARE(instance->property("labelFromContext").toString(), QStringLiteral("after"));
        QCOMPARE(instance->property("indexFromContext").toInt(), 0);
    }

    void scriptModelMovesStableKeysAndUpdatesPayloadWithoutReset()
    {
        QJSEngine engine;
        ScriptModel model;
        model.setKeyProperty(QStringLiteral("id"));

        QSignalSpy moved(&model, &QAbstractItemModel::rowsMoved);
        QSignalSpy reset(&model, &QAbstractItemModel::modelReset);
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

        model.setValues({
            jsRow(engine, QStringLiteral("a"), 1),
            jsRow(engine, QStringLiteral("b"), 2)
        });
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(reset.count(), 0);

        model.setValues({
            jsRow(engine, QStringLiteral("b"), 20),
            jsRow(engine, QStringLiteral("a"), 10)
        });

        QCOMPARE(model.rowCount(), 2);
        QVERIFY(moved.count() >= 1);
        QCOMPARE(reset.count(), 0);
        QVERIFY(changed.count() >= 1);
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("b"));
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("value")).toInt(), 20);
        QCOMPARE(model.get(1).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("a"));
        QCOMPARE(model.get(1).toMap().value(QStringLiteral("value")).toInt(), 10);

        model.setValues({
            jsRow(engine, QStringLiteral("duplicate"), 1),
            jsRow(engine, QStringLiteral("duplicate"), 2)
        });
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(reset.count(), 1);
    }

    void objectListModelRemovesDestroyedObjectsSafely()
    {
        ObjectListModel model;
        auto *object = new QObject;
        QVERIFY(model.append(object));
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.get(0), object);

        QSignalSpy removed(&model, &ObjectListModel::objectRemoved);
        delete object;

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(removed.count(), 1);
        QCOMPARE(removed.at(0).at(0).value<QObject *>(), nullptr);
    }
};

QTEST_MAIN(ModelsTests)
#include "tst_models.moc"
