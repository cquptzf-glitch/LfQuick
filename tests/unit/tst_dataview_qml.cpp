#include <QDebug>
#include <QStandardItemModel>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTest>

class DataViewQmlSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void hierarchicalTreeControlsAcceptAbstractItemModels()
    {
        QStandardItemModel model;
        model.setHorizontalHeaderLabels({QStringLiteral("Name")});
        auto *instrument = new QStandardItem(QStringLiteral("Instrument A"));
        instrument->appendRow(new QStandardItem(QStringLiteral("Channel 1")));
        instrument->appendRow(new QStandardItem(QStringLiteral("Channel 2")));
        model.appendRow(instrument);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.rowCount(model.index(0, 0)), 2);

        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("deviceTree"), &model);
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick.DataView

Item {
    width: 600
    height: 480

    LfTreeView {
        objectName: "treeView"
        width: 300
        height: 300
        model: deviceTree
    }
    LfTreeSelect {
        objectName: "treeSelect"
        y: 320
        model: deviceTree
        placeholderText: "Choose a channel"
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickTreeControls.qml")));

        if (component.isError()) {
            for (const QQmlError &error : component.errors())
                qWarning().noquote() << error.toString();
        }
        QObject *root = component.create();
        QVERIFY2(root, qPrintable(component.errorString()));
        QObject *tree = root->findChild<QObject *>(QStringLiteral("treeView"));
        QObject *picker = root->findChild<QObject *>(QStringLiteral("treeSelect"));
        QVERIFY(tree);
        QVERIFY(picker);
        QCOMPARE(picker->property("placeholderText").toString(),
                 QStringLiteral("Choose a channel"));
        delete root;
    }

    void publicDataViewTypesInstantiate()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick
import LfQuick.DataView

Item {
    width: 800
    height: 600

    DataGridColumn {
        id: column
        columnId: "sampleId"
        modelColumn: 0
        title: "Sample ID"
        width: 160
        minimumWidth: 80
    }

    DataGridViewport {
        id: viewport
        width: 400
        height: 240
        rowHeight: Units.controlHeight
        defaultColumnWidth: column.effectiveWidth()
    }

    DataGridHeader {
        width: viewport.width
        grid: null
    }

    DataGrid {
        id: grid
        anchors.fill: parent
    }

    Component { id: textEditor; DataGridEditorDelegate {} }
    Component { id: checkCell; DataGridCheckDelegate {} }
    Component { id: checkEditor; DataGridCheckEditorDelegate {} }
    Component { id: comboCell; DataGridComboDelegate {} }
    Component { id: comboEditor; DataGridComboEditorDelegate {} }
    Component { id: progressCell; DataGridProgressDelegate {} }
    Component { id: iconCell; DataGridIconDelegate {} }
    Component { id: badgeCell; DataGridBadgeDelegate {} }
    Component { id: buttonCell; DataGridButtonDelegate {} }
    Component { id: customCell; DataGridCustomDelegate {} }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickDataViewSmoke.qml")));

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

QTEST_MAIN(DataViewQmlSmokeTest)
#include "tst_dataview_qml.moc"
