#include <LfQuick/QtAdapters/RangeModelBridge.h>

#include <QAbstractItemModel>
#include <QTest>

using namespace LfQuick;

class RangeModelAdapterTest : public QObject
{
    Q_OBJECT

private slots:
    void bridgeExposesVariantMapRoles()
    {
        RangeModelBridge bridge;
        bridge.setRows({
            QVariantMap{{QStringLiteral("name"), QStringLiteral("Alpha")},
                        {QStringLiteral("value"), 10}},
            QVariantMap{{QStringLiteral("name"), QStringLiteral("Beta")},
                        {QStringLiteral("value"), 20}}
        });

        QAbstractItemModel *model = bridge.model();
        QVERIFY(model);
        QCOMPARE(model->rowCount(), 2);
        QVERIFY(bridge.roleKeys().contains(QStringLiteral("name")));
        QVERIFY(bridge.roleKeys().contains(QStringLiteral("value")));

        int nameRole = -1;
        int valueRole = -1;
        const auto roles = model->roleNames();
        for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
            if (it.value() == QByteArrayLiteral("name"))
                nameRole = it.key();
            if (it.value() == QByteArrayLiteral("value"))
                valueRole = it.key();
        }
        QVERIFY(nameRole >= Qt::UserRole);
        QVERIFY(valueRole >= Qt::UserRole);

        QCOMPARE(model->data(model->index(0, 0), nameRole).toString(), QStringLiteral("Alpha"));
        QCOMPARE(model->data(model->index(1, 0), valueRole).toInt(), 20);

        QVERIFY(model->setData(model->index(0, 0), QStringLiteral("Updated"), nameRole));
        QCOMPARE(bridge.rowAt(0).value(QStringLiteral("name")).toString(), QStringLiteral("Updated"));
    }

    void dragDropFeatureTracksQtVersion()
    {
        RangeModelBridge bridge;
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
        QVERIFY(bridge.dragDropSupported());
        QVERIFY(bridge.setDragDropEnabled(true));
        QVERIFY(bridge.dragDropEnabled());
        QVERIFY(bridge.model()->supportedDragActions().testFlag(Qt::MoveAction));
        QVERIFY(bridge.model()->supportedDropActions().testFlag(Qt::MoveAction));
#else
        QVERIFY(!bridge.dragDropSupported());
        QVERIFY(!bridge.setDragDropEnabled(true));
        QVERIFY(!bridge.dragDropEnabled());
#endif
    }
};

QTEST_MAIN(RangeModelAdapterTest)
#include "tst_range_model_adapter.moc"
