#include <LfQuick/DataView/DataFilterProxyModel.h>
#include <LfQuick/DataView/DataTableModel.h>

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
};

QTEST_MAIN(DataViewTest)
#include "tst_dataview.moc"
