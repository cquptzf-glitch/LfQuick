#include <LfQuick/DataView/ColumnConfigStore.h>
#include <LfQuick/DataView/ColumnStateModel.h>
#include <LfQuick/DataView/DataEditTransaction.h>
#include <LfQuick/DataView/DataFilterProxyModel.h>
#include <LfQuick/DataView/DataGridSelectionController.h>
#include <LfQuick/DataView/DataTableModel.h>

#include <QItemSelectionModel>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace LfQuick;

class DataViewTest : public QObject
{
    Q_OBJECT

private slots:
    void tableModelSupportsRolesEditingAndLookup()
    {
        DataTableModel model;
        model.setColumnKeys({QStringLiteral("sampleId"), QStringLiteral("status"), QStringLiteral("result")});
        model.appendRow({
            {QStringLiteral("sampleId"), QStringLiteral("S-001")},
            {QStringLiteral("status"), QStringLiteral("Running")},
            {QStringLiteral("result"), 12.5}
        });

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.columnCount(), 3);
        model.setColumnTitles({QStringLiteral("Sample ID"), QStringLiteral("Status"), QStringLiteral("Result")});
        QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), QStringLiteral("Sample ID"));
        QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("S-001"));
        QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toString(), QStringLiteral("Running"));

        const auto roles = model.roleNames();
        QVERIFY(roles.values().contains(QByteArrayLiteral("status")));
        QVERIFY(model.setCell(0, QStringLiteral("status"), QStringLiteral("Completed")));
        QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toString(), QStringLiteral("Completed"));

        QCOMPARE(model.findFirst(QStringLiteral("sampleId"), QStringLiteral("S-001")), 0);
        QCOMPARE(model.findFirst(QStringLiteral("sampleId"), QStringLiteral("s-001")), -1);
        QCOMPARE(model.findFirst(QStringLiteral("sampleId"), QStringLiteral("s-001"), Qt::CaseInsensitive), 0);
        QCOMPARE(model.findFirst(QStringLiteral("missing"), QStringLiteral("S-001")), -1);
    }

    void proxyFiltersAcrossColumnsAndMapsRows()
    {
        DataTableModel model;
        model.setColumnKeys({QStringLiteral("sampleId"), QStringLiteral("status")});
        model.appendRow({{QStringLiteral("sampleId"), QStringLiteral("S-001")},
                         {QStringLiteral("status"), QStringLiteral("Running")}});
        model.appendRow({{QStringLiteral("sampleId"), QStringLiteral("S-002")},
                         {QStringLiteral("status"), QStringLiteral("Completed")}});

        DataFilterProxyModel proxy;
        proxy.setSourceModel(&model);
        proxy.setFilterText(QStringLiteral("completed"));

        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("S-002"));
        QCOMPARE(proxy.mapRowToSource(0), 1);
        QCOMPARE(proxy.mapRowFromSource(1), 0);
        QCOMPARE(proxy.mapRowFromSource(0), -1);
        QCOMPARE(proxy.mapRowToSource(10), -1);

        proxy.setFilterKey(QStringLiteral("sampleId"));
        proxy.setFilterText(QStringLiteral("001"));
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.mapRowToSource(0), 0);
        QCOMPARE(proxy.mapRowFromSource(0), 0);

        proxy.setFilterKey(QStringLiteral("status"));
        proxy.setExactMatch(true);
        proxy.setFilterText(QStringLiteral("Completed"));
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.mapRowToSource(0), 1);
        proxy.setFilterText(QStringLiteral("Complete"));
        QCOMPARE(proxy.rowCount(), 0);
    }

    void columnStateRestoresByStableIdNotOldIndex()
    {
        ColumnStateModel original;
        QVERIFY(original.setColumns({
            QVariantMap{{QStringLiteral("id"), QStringLiteral("sample")}, {QStringLiteral("modelColumn"), 0}, {QStringLiteral("width"), 120}},
            QVariantMap{{QStringLiteral("id"), QStringLiteral("status")}, {QStringLiteral("modelColumn"), 1}, {QStringLiteral("width"), 100}},
            QVariantMap{{QStringLiteral("id"), QStringLiteral("result")}, {QStringLiteral("modelColumn"), 2}, {QStringLiteral("width"), 90}}
        }));
        QVERIFY(original.setWidth(QStringLiteral("status"), 188));
        QVERIFY(original.setVisible(QStringLiteral("result"), false));
        QVERIFY(original.moveModelColumn(2, 0));
        const QVariantList saved = original.snapshot();

        ColumnStateModel changedSchema;
        QVERIFY(changedSchema.setColumns({
            QVariantMap{{QStringLiteral("id"), QStringLiteral("status")}, {QStringLiteral("modelColumn"), 0}, {QStringLiteral("width"), 100}},
            QVariantMap{{QStringLiteral("id"), QStringLiteral("sample")}, {QStringLiteral("modelColumn"), 1}, {QStringLiteral("width"), 120}},
            QVariantMap{{QStringLiteral("id"), QStringLiteral("result")}, {QStringLiteral("modelColumn"), 2}, {QStringLiteral("width"), 90}},
            QVariantMap{{QStringLiteral("id"), QStringLiteral("flag")}, {QStringLiteral("modelColumn"), 3}, {QStringLiteral("width"), 80}}
        }));

        QVERIFY(changedSchema.restoreState(saved));
        QCOMPARE(changedSchema.modelColumnAt(0), 2);
        QCOMPARE(changedSchema.idForModelColumn(0), QStringLiteral("status"));
        QCOMPARE(changedSchema.titleForModelColumn(3), QStringLiteral("flag"));
        QCOMPARE(changedSchema.widthForModelColumn(0), 188.0);
        QVERIFY(!changedSchema.isModelColumnVisible(2));
        QCOMPARE(changedSchema.modelColumnAt(3), 3);
    }

    void columnConfigStoreRoundTripsAndRejectsSchemaMismatch()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        ColumnConfigStore store;
        store.setFilePath(temporary.filePath(QStringLiteral("columns.ini")));
        const QVariantList columns{
            QVariantMap{{QStringLiteral("id"), QStringLiteral("sample")}, {QStringLiteral("modelColumn"), 0}, {QStringLiteral("width"), 160.0}, {QStringLiteral("visible"), true}, {QStringLiteral("order"), 0}}
        };

        QVERIFY(store.save(QStringLiteral("results"), 2, QStringLiteral("ResultModel.v3"), columns));
        const QVariantMap restored = store.load(QStringLiteral("results"), 2, QStringLiteral("ResultModel.v3"));
        QVERIFY(restored.value(QStringLiteral("matched")).toBool());
        QCOMPARE(restored.value(QStringLiteral("columns")).toList().size(), 1);
        QVERIFY(!store.load(QStringLiteral("results"), 3, QStringLiteral("ResultModel.v3")).value(QStringLiteral("matched")).toBool());
        QVERIFY(!store.load(QStringLiteral("results"), 2, QStringLiteral("ResultModel.v4")).value(QStringLiteral("matched")).toBool());
    }

    void editTransactionCommitsOnlyAfterValidation()
    {
        DataTableModel model;
        model.setColumnKeys({QStringLiteral("sample"), QStringLiteral("result")});
        model.appendRow({{QStringLiteral("sample"), QStringLiteral("S-001")},
                         {QStringLiteral("result"), 12.5}});

        DataEditTransaction transaction;
        transaction.setModel(&model);
        QSignalSpy committed(&transaction, &DataEditTransaction::committed);

        QVERIFY(transaction.begin(0, 1));
        QCOMPARE(transaction.originalValue().toDouble(), 12.5);
        transaction.setTemporaryValue(13.8);
        QVERIFY(!transaction.commit());
        QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toDouble(), 12.5);

        QVERIFY(transaction.acceptValidation(true));
        QVERIFY(transaction.commit());
        QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toDouble(), 13.8);
        QCOMPARE(committed.count(), 1);
    }

    void editTransactionRollbackAndValidationFailureLeaveModelUntouched()
    {
        DataTableModel model;
        model.setColumnKeys({QStringLiteral("status")});
        model.appendRow({{QStringLiteral("status"), QStringLiteral("Pending")}});

        DataEditTransaction transaction;
        transaction.setModel(&model);
        QVERIFY(transaction.begin(0, 0));
        transaction.setTemporaryValue(QStringLiteral("Invalid"));
        QVERIFY(!transaction.acceptValidation(false, QStringLiteral("Unsupported state")));
        QVERIFY(!transaction.commit());
        QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Pending"));

        transaction.rollback();
        QVERIFY(!transaction.active());
        QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Pending"));
    }

    void selectionControllerSupportsBatchRowsAndCurrentCell()
    {
        DataTableModel model;
        model.setColumnKeys({QStringLiteral("id"), QStringLiteral("status")});
        for (int i = 0; i < 4; ++i) {
            model.appendRow({{QStringLiteral("id"), QString::number(i)},
                             {QStringLiteral("status"), QStringLiteral("Ready")}});
        }

        QItemSelectionModel selection(&model);
        DataGridSelectionController controller;
        controller.setSelectionModel(&selection);
        controller.setSelectRows(true);

        QVERIFY(controller.selectRowsList({0, 2}));
        QCOMPARE(controller.selectedRowsList(), QVariantList({0, 2}));
        QCOMPARE(controller.selectedRowCount(), 2);
        QVERIFY(controller.isRowSelected(2));

        QVERIFY(controller.setCurrentCell(1, 1));
        QCOMPARE(controller.currentRow(), 1);
        QCOMPARE(controller.currentColumn(), 1);
        QCOMPARE(controller.selectedRowsList(), QVariantList{QVariant(1)});

        QVERIFY(controller.moveCurrent(2, 0));
        QCOMPARE(controller.currentRow(), 3);
        QVERIFY(controller.selectAllRows());
        QCOMPARE(controller.selectedRowCount(), 4);

        controller.clear();
        QCOMPARE(controller.selectedRowCount(), 0);
        QCOMPARE(controller.currentRow(), -1);
    }
};

QTEST_MAIN(DataViewTest)
#include "tst_dataview.moc"
