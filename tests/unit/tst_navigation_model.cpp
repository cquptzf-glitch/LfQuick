#include <LfQuick/Core/Permission.h>
#include <LfQuick/Ui/NavigationController.h>
#include <LfQuick/Ui/NavigationItemModel.h>
#include <LfQuick/Ui/PermissionEvaluator.h>

#include <QSignalSpy>
#include <QtTest>

#include <memory>

namespace {

class StaticPermissionProvider final : public LfQuick::IPermissionProvider
{
public:
    explicit StaticPermissionProvider(LfQuick::PermissionState state)
        : m_state(state) {}

    LfQuick::PermissionState evaluate(QStringView,
                                      const QVariantMap &) const override
    {
        return m_state;
    }

private:
    LfQuick::PermissionState m_state;
};

} // namespace

class NavigationModelTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesRouteMetadataAndSelection()
    {
        LfQuick::NavigationItemModel model;
        QVERIFY(model.appendItem({
            {QStringLiteral("route"), QStringLiteral("/dashboard")},
            {QStringLiteral("label"), QStringLiteral("Dashboard")},
            {QStringLiteral("section"), QStringLiteral("primary")},
            {QStringLiteral("group"), QStringLiteral("Workspace")},
            {QStringLiteral("badge"), 3},
            {QStringLiteral("parameters"), QVariantMap{{QStringLiteral("mode"), QStringLiteral("summary")}}}
        }));
        QVERIFY(model.appendItem({
            {QStringLiteral("route"), QStringLiteral("/settings")},
            {QStringLiteral("label"), QStringLiteral("Settings")},
            {QStringLiteral("section"), QStringLiteral("footer")}
        }));
        QVERIFY(!model.appendItem({
            {QStringLiteral("route"), QStringLiteral("/dashboard")},
            {QStringLiteral("label"), QStringLiteral("Duplicate")}
        }));

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.rowForRoute(QStringLiteral("/settings")), 1);
        QCOMPARE(model.get(0).value(QStringLiteral("group")).toString(), QStringLiteral("Workspace"));
        QCOMPARE(model.get(0).value(QStringLiteral("badge")).toInt(), 3);
        QCOMPARE(model.get(1).value(QStringLiteral("section")).toString(), QStringLiteral("footer"));

        model.setSelectedRoute(QStringLiteral("/settings"));
        QVERIFY(model.get(1).value(QStringLiteral("selected")).toBool());
        QVERIFY(!model.get(0).value(QStringLiteral("selected")).toBool());
    }

    void permissionItemsFailClosedAndRefreshWithProvider()
    {
        LfQuick::NavigationItemModel model;
        LfQuick::PermissionEvaluator evaluator;
        model.setPermissionEvaluator(&evaluator);

        QVERIFY(model.appendItem({
            {QStringLiteral("route"), QStringLiteral("/developer")},
            {QStringLiteral("label"), QStringLiteral("Developer")},
            {QStringLiteral("permissionId"), QStringLiteral("developer.view")}
        }));

        QVERIFY(!model.get(0).value(QStringLiteral("visible")).toBool());
        QVERIFY(!model.get(0).value(QStringLiteral("enabled")).toBool());

        evaluator.setProvider(std::make_shared<StaticPermissionProvider>(LfQuick::PermissionState::Allow));
        QVERIFY(model.get(0).value(QStringLiteral("visible")).toBool());
        QVERIFY(model.get(0).value(QStringLiteral("enabled")).toBool());

        evaluator.setProvider(std::make_shared<StaticPermissionProvider>(LfQuick::PermissionState::Disabled));
        QVERIFY(model.get(0).value(QStringLiteral("visible")).toBool());
        QVERIFY(!model.get(0).value(QStringLiteral("enabled")).toBool());

        evaluator.setProvider(std::make_shared<StaticPermissionProvider>(LfQuick::PermissionState::Hidden));
        QVERIFY(!model.get(0).value(QStringLiteral("visible")).toBool());
    }

    void setItemsSkipsInvalidAndDuplicateRoutes()
    {
        LfQuick::NavigationItemModel model;
        model.setSelectedRoute(QStringLiteral("/missing"));
        model.setItems({
            QVariantMap{{QStringLiteral("route"), QStringLiteral("/a")},
                        {QStringLiteral("label"), QStringLiteral("A")}},
            QVariantMap{{QStringLiteral("route"), QStringLiteral("/a")},
                        {QStringLiteral("label"), QStringLiteral("Duplicate")}},
            QVariantMap{{QStringLiteral("label"), QStringLiteral("Missing route")}},
            QVariantMap{{QStringLiteral("route"), QStringLiteral("/b")},
                        {QStringLiteral("label"), QStringLiteral("B")}}
        });

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.get(0).value(QStringLiteral("route")).toString(), QStringLiteral("/a"));
        QCOMPARE(model.get(1).value(QStringLiteral("route")).toString(), QStringLiteral("/b"));
        QVERIFY(model.selectedRoute().isEmpty());
    }

    void controllerNotifiesWhenModelIsDestroyed()
    {
        LfQuick::NavigationController controller;
        auto *model = new LfQuick::NavigationItemModel;
        controller.setModel(model);
        QCOMPARE(controller.model(), model);

        QSignalSpy changed(&controller, &LfQuick::NavigationController::modelChanged);
        delete model;

        QCOMPARE(controller.model(), nullptr);
        QCOMPARE(changed.count(), 1);
    }
};

QTEST_MAIN(NavigationModelTests)
#include "tst_navigation_model.moc"
