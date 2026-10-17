#include <LfQuick/Ui/LifetimeLease.h>

#include <QDebug>
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QPointer>
#include <QQuickItem>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantMap>

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

    LfLifetimeLease { object: parent }
    LfObjectFactory { }
    LfLazyLoader { }
    LfBoundComponent { }

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

        QSignalSpy itemChanged(loader, SIGNAL(itemChanged()));
        QSignalSpy activeChanged(loader, SIGNAL(activeChanged()));

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

        // Creation inputs are transactional: changing initialProperties while
        // active must rebuild the item rather than leave the property and the
        // actual constructed object out of sync.
        QPointer<QObject> firstItem = loaded;
        const int itemSignalsBeforeRecreate = itemChanged.count();
        QVERIFY(loader->setProperty(
            "initialProperties",
            QVariantMap{{QStringLiteral("answer"), 23}}));
        QObject *recreated = loader->property("item").value<QObject *>();
        QVERIFY(recreated);
        QVERIFY(recreated != firstItem.data());
        QCOMPARE(recreated->property("answer").toInt(), 23);
        QVERIFY(loader->property("active").toBool());
        QVERIFY(itemChanged.count() >= itemSignalsBeforeRecreate + 2);

        // If loaded content destroys itself, the loader must not silently leave
        // QML bindings with stale active/item state.
        const int itemSignalsBeforeDestroy = itemChanged.count();
        const int activeSignalsBeforeDestroy = activeChanged.count();
        delete recreated;
        QCOMPARE(loader->property("item").value<QObject *>(), nullptr);
        QVERIFY(!loader->property("active").toBool());
        QCOMPARE(itemChanged.count(), itemSignalsBeforeDestroy + 1);
        QCOMPARE(activeChanged.count(), activeSignalsBeforeDestroy + 1);

        // The loader remains reusable after unexpected content destruction.
        QVERIFY(loader->setProperty("active", true));
        QObject *reloaded = loader->property("item").value<QObject *>();
        QVERIFY(reloaded);
        QCOMPARE(reloaded->property("answer").toInt(), 23);

        QVERIFY(loader->setProperty("active", false));
        QCOMPARE(loader->property("item").value<QObject *>(), nullptr);

        delete root;
    }

    void boundComponentPreservesRetainedVisualContentAndTracksDestruction()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

LfBoundComponent {
    objectName: "bound"
    width: 120
    height: 60
    asynchronous: false
    property int answer: 17

    component: Component {
        Item {
            required property int answer
            implicitWidth: 80
            implicitHeight: 30
        }
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBoundComponent.qml")));

        QObject *host = component.create();
        QVERIFY2(host, qPrintable(component.errorString()));
        auto *hostItem = qobject_cast<QQuickItem *>(host);
        QVERIFY(hostItem);

        QObject *first = host->property("item").value<QObject *>();
        auto *firstItem = qobject_cast<QQuickItem *>(first);
        QVERIFY(firstItem);
        QCOMPARE(first->property("answer").toInt(), 17);
        QCOMPARE(host->property("implicitWidth").toDouble(), 80.0);
        QCOMPARE(host->property("implicitHeight").toDouble(), 30.0);

        QVERIFY(host->setProperty("answer", 23));
        QCOMPARE(first->property("answer").toInt(), 23);

        LfQuick::LifetimeLease lease;
        lease.setObject(first);
        lease.setHeld(true);
        QPointer<QObject> retained = first;
        QSignalSpy itemChanged(host, SIGNAL(itemChanged()));

        QVERIFY(QMetaObject::invokeMethod(host, "clear", Qt::DirectConnection));
        QCOMPARE(host->property("item").value<QObject *>(), nullptr);
        QVERIFY(retained);
        QCOMPARE(retained->parent(), nullptr);
        QCOMPARE(firstItem->parentItem(), hostItem);
        QVERIFY(LfQuick::LifetimeLease::isRetained(retained));
        QCOMPARE(host->property("implicitWidth").toDouble(), 0.0);
        QCOMPARE(host->property("implicitHeight").toDouble(), 0.0);

        lease.setHeld(false);
        QTRY_VERIFY(retained.isNull());

        // clear() keeps the component definition, so reload remains available.
        QVERIFY(QMetaObject::invokeMethod(host, "reload", Qt::DirectConnection));
        QObject *second = host->property("item").value<QObject *>();
        QVERIFY(second);
        QCOMPARE(second->property("answer").toInt(), 23);

        const int itemSignalsBeforeDestroy = itemChanged.count();
        delete second;
        QCOMPARE(host->property("item").value<QObject *>(), nullptr);
        QCOMPARE(host->property("implicitWidth").toDouble(), 0.0);
        QCOMPARE(host->property("implicitHeight").toDouble(), 0.0);
        QCOMPARE(itemChanged.count(), itemSignalsBeforeDestroy + 1);

        delete host;
    }
};

QTEST_MAIN(UiControlsSmokeTest)
#include "tst_ui_controls.moc"
