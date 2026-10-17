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
    LfToolButton { text: "Tool"; checkable: true; checked: true }
    LfMenuButton { text: "More" }
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
    LfBadge { tone: LfBadge.Success; text: "Online" }
    LfInfoBar {
        severity: LfInfoBar.Warning
        title: "Warning"
        message: "Component smoke test"
        closable: true
    }
    LfDivider { }
    LfDivider { orientation: LfDivider.Vertical }
    LfCard {
        title: "Card"
        subtitle: "Composable content"
        LfButton { text: "Action" }
    }
    LfTabBar {
        LfTabButton { text: "Overview" }
        LfTabButton { text: "Details" }
    }
    LfBreadcrumb { model: ["Home", "Samples", "Review"] }
    LfPagination { pageCount: 12; currentPage: 3 }
    LfExpander {
        title: "Details"
        expanded: true
        LfButton { text: "Inside" }
    }
    LfEmptyState {
        title: "No samples"
        message: "Register a sample to continue."
        actionText: "Register"
    }
    LfSkeleton { width: 180; running: false }
    LfAvatar { name: "Lf Quick" }
    LfTag { text: "Priority"; tone: LfBadge.Warning; closable: true }
    LfCommandBar {
        LfToolButton { text: "Refresh" }
        LfButton { text: "Run"; highlighted: true }
    }
    LfSplitView {
        width: 320
        height: 120
        Item { }
        Item { }
    }
    LfFormField {
        label: "Sample ID"
        description: "Required identifier"
        required: true
        LfTextField { text: "S-001" }
    }
    LfFormSection {
        title: "General"
        description: "Application settings"
        framed: true
        LfSettingsItem {
            title: "Notifications"
            description: "Show runtime notifications"
            LfSwitch { checked: true }
        }
    }
    LfListItem {
        title: "Sample A"
        subtitle: "Ready for review"
        leadingText: "A"
        trailingText: "10:30"
        selected: true
    }
    LfStepper {
        model: ["Prepare", "Run", "Review"]
        currentIndex: 1
    }
    LfTimeline {
        width: 320
        model: [
            { title: "Queued", description: "Sample accepted", detail: "10:00" },
            { title: "Running", description: "Analysis in progress", detail: "10:05" }
        ]
        currentIndex: 1
    }
    LfSideSheet {
        title: "Details"
        LfButton { text: "Sheet action" }
    }
    LfContextMenu {
        model: ["Open", { text: "Disabled", enabled: false }]
    }
    LfTransferList {
        model: [{ text: "One", value: "1" }, { text: "Two", value: "2" }]
        targetValues: ["1"]
        width: 500
        height: 240
    }
    LfPropertyGrid {
        model: [
            { key: "name", label: "Name", value: "Analyzer", type: "text" },
            { key: "enabled", label: "Enabled", value: true, type: "bool" },
            { key: "threshold", label: "Threshold", value: 40, type: "int", from: 0, to: 100 }
        ]
    }
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

    void infoBarCloseIsObservableAndReusable()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

LfInfoBar {
    objectName: "infoBar"
    title: "Connection"
    message: "Disconnected"
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickInfoBar.qml")));

        QObject *infoBar = component.create();
        QVERIFY2(infoBar, qPrintable(component.errorString()));
        QSignalSpy closeRequested(infoBar, SIGNAL(closeRequested()));

        QVERIFY(infoBar->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(infoBar, "close", Qt::DirectConnection));
        QCOMPARE(closeRequested.count(), 1);
        QVERIFY(infoBar->property("closed").toBool());
        QVERIFY(!infoBar->property("visible").toBool());

        QVERIFY(infoBar->setProperty("closed", false));
        QVERIFY(infoBar->property("visible").toBool());
        delete infoBar;
    }

    void paginationAndExpanderStateAreObservable()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

Item {
    LfPagination {
        objectName: "pagination"
        pageCount: 10
        currentPage: 1
        property int lastRequested: 0
        onPageRequested: function(page) { lastRequested = page }
        Component.onCompleted: requestPage(7)
    }

    LfExpander {
        objectName: "expander"
        title: "Advanced"
        property int toggleCount: 0
        onToggled: function(value) { toggleCount += 1 }
        Component.onCompleted: toggle()
        LfButton { text: "Action" }
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBatch3State.qml")));

        QObject *root = component.create();
        QVERIFY2(root, qPrintable(component.errorString()));

        QObject *pagination = root->findChild<QObject *>(QStringLiteral("pagination"));
        QObject *expander = root->findChild<QObject *>(QStringLiteral("expander"));
        QVERIFY(pagination);
        QVERIFY(expander);

        QCOMPARE(pagination->property("currentPage").toInt(), 7);
        QCOMPARE(pagination->property("lastRequested").toInt(), 7);
        QVERIFY(expander->property("expanded").toBool());
        QCOMPARE(expander->property("toggleCount").toInt(), 1);

        delete root;
    }

    void stepperAndContextMenuContractsAreObservable()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

Item {
    LfStepper {
        objectName: "stepper"
        model: ["Prepare", "Run", "Review"]
        currentIndex: 0
        property int lastIndex: -1
        property string lastValue: ""
        onStepActivated: function(index, value) {
            lastIndex = index
            lastValue = String(value)
        }
        Component.onCompleted: activate(2)
    }

    LfContextMenu {
        objectName: "contextMenu"
        model: ["Open", { text: "Disabled", enabled: false }]
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBatch4State.qml")));

        QObject *root = component.create();
        QVERIFY2(root, qPrintable(component.errorString()));

        QObject *stepper = root->findChild<QObject *>(QStringLiteral("stepper"));
        QObject *menu = root->findChild<QObject *>(QStringLiteral("contextMenu"));
        QVERIFY(stepper);
        QVERIFY(menu);

        QCOMPARE(stepper->property("currentIndex").toInt(), 2);
        QCOMPARE(stepper->property("lastIndex").toInt(), 2);
        QCOMPARE(stepper->property("lastValue").toString(), QStringLiteral("Review"));
        QTRY_COMPARE(menu->property("count").toInt(), 2);

        delete root;
    }

    void complexSelectorsPreserveStableValues()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import QtQml.Models
import LfQuick

Item {
    LfMultiSelect {
        id: multi
        objectName: "multi"
        model: [
            { text: "Priority", value: "priority" },
            { text: "Quality control", value: "qc" },
            { text: "Disabled", value: "off", enabled: false }
        ]
        maximumSelection: 1
        property int commitCount: 0
        property bool secondRejected: false
        property bool disabledRejected: false
        onSelectionCommitted: function(keys) { commitCount += 1 }
        Component.onCompleted: {
            toggleIndex(0)
            secondRejected = !toggleIndex(1)
            disabledRejected = !toggleIndex(2)
        }
    }

    ListModel {
        id: listOptions
        ListElement { text: "First"; value: "first" }
        ListElement { text: "Second"; value: "second" }
    }
    LfMultiSelect {
        objectName: "modelMulti"
        model: listOptions
        property bool toggledFromModel: false
        Component.onCompleted: toggledFromModel = toggleIndex(1)
    }

    LfSearchableComboBox {
        objectName: "searchable"
        model: ["Alpha", "Beta", "Gamma"]
        searchText: "bet"
        readonly property int filteredCount: filteredIndices.length
        readonly property int firstSourceIndex: filteredIndices.length > 0 ? filteredIndices[0] : -1
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBatch5Selectors.qml")));

        QObject *root = component.create();
        QVERIFY2(root, qPrintable(component.errorString()));
        QObject *multi = root->findChild<QObject *>(QStringLiteral("multi"));
        QObject *searchable = root->findChild<QObject *>(QStringLiteral("searchable"));
        QVERIFY(multi);
        QVERIFY(searchable);

        const QVariantList values = multi->property("selectedValues").toList();
        QCOMPARE(values.size(), 1);
        QCOMPARE(values.first().toString(), QStringLiteral("priority"));
        QCOMPARE(multi->property("commitCount").toInt(), 1);
        QVERIFY(multi->property("secondRejected").toBool());
        QVERIFY(multi->property("disabledRejected").toBool());
        QCOMPARE(searchable->property("filteredCount").toInt(), 1);
        QCOMPARE(searchable->property("firstSourceIndex").toInt(), 1);
        QObject *modelMulti = root->findChild<QObject *>(QStringLiteral("modelMulti"));
        QVERIFY(modelMulti);
        QVERIFY(modelMulti->property("toggledFromModel").toBool());
        const QVariantList modelKeys = modelMulti->property("selectedValues").toList();
        QCOMPARE(modelKeys.size(), 1);
        QCOMPARE(modelKeys.first().toString(), QStringLiteral("second"));
        delete root;
    }

    void transferListMaintainsUniqueStableKeys()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

LfTransferList {
    model: [
        { text: "A", value: "a" },
        { text: "B", value: "b" },
        { text: "Disabled", value: "c", enabled: false }
    ]
    property bool rejectedDuplicate: false
    property bool rejectedDisabled: false
    property int changes: 0
    onItemsTransferred: function(values) { changes += 1 }
    Component.onCompleted: {
        addIndex(0)
        rejectedDuplicate = !addIndex(0)
        rejectedDisabled = !addIndex(2)
        addIndex(1)
        removeKey("a")
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickTransferList.qml")));

        QObject *transfer = component.create();
        QVERIFY2(transfer, qPrintable(component.errorString()));
        const QVariantList keys = transfer->property("targetValues").toList();
        QCOMPARE(keys.size(), 1);
        QCOMPARE(keys.first().toString(), QStringLiteral("b"));
        QCOMPARE(transfer->property("changes").toInt(), 3);
        QVERIFY(transfer->property("rejectedDuplicate").toBool());
        QVERIFY(transfer->property("rejectedDisabled").toBool());
        delete transfer;
    }

    void propertyGridEmitsEditsAndHonorsReadOnly()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

LfPropertyGrid {
    objectName: "grid"
    model: [
        { key: "threshold", label: "Threshold", value: 3, type: "int" },
        { key: "serial", label: "Serial", value: "X", readOnly: true }
    ]
    property int edits: 0
    property int accepted: 0
    property int rejected: 0
    property string editedKey: ""
    property int editedValue: -1

    onValueEdited: function(index, key, value) {
        ++edits
        editedKey = key
        editedValue = value
    }
    Component.onCompleted: {
        if (editValue(0, 4))
            ++accepted
        if (!editValue(1, "Y"))
            ++rejected
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickPropertyGrid.qml")));

        QObject *grid = component.create();
        QVERIFY2(grid, qPrintable(component.errorString()));
        QCOMPARE(grid->property("entryCount").toInt(), 2);
        QCOMPARE(grid->property("edits").toInt(), 1);
        QCOMPARE(grid->property("accepted").toInt(), 1);
        QCOMPARE(grid->property("rejected").toInt(), 1);
        QCOMPARE(grid->property("editedKey").toString(), QStringLiteral("threshold"));
        QCOMPARE(grid->property("editedValue").toInt(), 4);
        delete grid;
    }

    void searchableComboFiltersAfterComponentCompletion()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

LfSearchableComboBox {
    model: ["Alpha", "Beta", "Gamma"]
    property int filteredCount: filteredIndices.length
    Component.onCompleted: searchText = "et"
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickSearchableCombo.qml")));
        QObject *combo = component.create();
        QVERIFY2(combo, qPrintable(component.errorString()));
        QCOMPARE(combo->property("filteredCount").toInt(), 1);
        QVERIFY(combo->property("_componentReady").toBool());
        delete combo;
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

        QObject *loaded = loader->property("item").value<QObject *>();
        QVERIFY2(loaded, qPrintable(loader->property("errorString").toString()));
        QCOMPARE(loaded->property("answer").toInt(), 17);
        QVERIFY(loader->property("active").toBool());

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

        const int itemSignalsBeforeDestroy = itemChanged.count();
        const int activeSignalsBeforeDestroy = activeChanged.count();
        delete recreated;
        QCOMPARE(loader->property("item").value<QObject *>(), nullptr);
        QVERIFY(!loader->property("active").toBool());
        QCOMPARE(itemChanged.count(), itemSignalsBeforeDestroy + 1);
        QCOMPARE(activeChanged.count(), activeSignalsBeforeDestroy + 1);

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
