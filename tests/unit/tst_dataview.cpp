#include <LfQuick/DataView/ColumnConfigStore.h>
#include <LfQuick/DataView/ColumnStateModel.h>
#include <LfQuick/DataView/DataFilterProxyModel.h>
#include <LfQuick/DataView/DataTableModel.h>

#include <QTemporaryDir>
#include <QTest>

using namespace LfQuick;

class DataViewTest : public QObject
{
    Q_OBJECT

private slots:
    void tableModelSupportsRolesAndEditing()
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
    }

    void proxyFiltersAcrossColumns()
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

        proxy.setFilterKey(QStringLiteral("sampleId"));
        proxy.setFilterText(QStringLiteral("001"));
        QCOMPARE(proxy.rowCount(), 1);
    }

    void columnStateRestoresByStableIdNotOldIndex()
    {
        ColumnStateModel original;
        QVERIFY(original.setColumns({
            QVariantMap{{QStringLiteral("id"), QStringLiteral("sample" )}, {QStringLiteral("modelColumn"), 0}, {QStringLiteral("width"), 120}},
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
        QCOMPARE(changedSchema.modelColumnAt(0), 2); // result keeps saved visual order by id
        QCOMPARE(changedSchema.widthForModelColumn(0), 188.0); // status moved to model column 0
        QVERIFY(!changedSchema.isModelColumnVisible(2));
        QCOMPARE(changedSchema.modelColumnAt(3), 3); // new column remains after restored known columns
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

        const QVariantMap stale = store.load(QStringLiteral("results"), 3, QStringLiteral("ResultModel.v3"));
        QVERIFY(!stale.value(QStringLiteral("matched")).toBool());
        const QVariantMap wrongModel = store.load(QStringLiteral("results"), 2, QStringLiteral("ResultModel.v4"));
        QVERIFY(!wrongModel.value(QStringLiteral("matched")).toBool());
    }
};

QTEST_MAIN(DataViewTest)
#include "tst_dataview.moc"
