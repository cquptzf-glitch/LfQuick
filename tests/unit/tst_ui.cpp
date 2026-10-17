#include <LfQuick/Ui/NavigationGuardController.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/PermissionEvaluator.h>
#include <LfQuick/Ui/QmlErrorCollector.h>
#include <LfQuick/Ui/QuickRuntime.h>
#include <LfQuick/Ui/ValidationEvaluator.h>

#include <LfQuick/Core/OperationHandle.h>
#include <LfQuick/Core/Permission.h>
#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>

#include <QQuickItem>
#include <QSignalSpy>
#include <QTest>
#include <QQmlEngine>
#include <QUrl>

#include <memory>

using namespace LfQuick;

namespace {

class TestPage final : public IPageInstance
{
public:
    explicit TestPage(QString id) : m_id(std::move(id)) {}
    QString pageId() const override { return m_id; }
    void activate() override {}
    void deactivate() override {}
    void suspend() override {}
    void resume() override {}
    void destroy() override {}

private:
    QString m_id;
};

class TestProvider final : public IPageProvider
{
public:
    QString id() const override { return QStringLiteral("guard-test"); }
    bool canCreate(const PageDescriptor &descriptor) const override
    {
        return descriptor.provider == id();
    }
    std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                           const QVariantMap &parameters) override
    {
        lastParameters = parameters;
        return std::make_unique<TestPage>(descriptor.route);
    }

    QVariantMap lastParameters;
};

class TestPermissionProvider final : public IPermissionProvider
{
public:
    PermissionState evaluate(QStringView permissionId,
                             const QVariantMap &context) const override
    {
        if (permissionId != QStringView(u"test.permission"))
            return PermissionState::Denied;

        const QString role = context.value(QStringLiteral("role")).toString();
        if (role == QStringLiteral("reviewer"))
            return PermissionState::Allow;
        if (role == QStringLiteral("operator"))
            return PermissionState::ReadOnly;
        if (role == QStringLiteral("hidden"))
            return PermissionState::Hidden;
        if (role == QStringLiteral("disabled"))
            return PermissionState::Disabled;
        return PermissionState::Denied;
    }
};

PageDescriptor guardTestPage(const QString &route)
{
    PageDescriptor descriptor;
    descriptor.route = route;
    descriptor.provider = QStringLiteral("guard-test");
    descriptor.cachePolicy = PageCachePolicy::KeepAlive;
    descriptor.fallbackPolicy = FallbackPolicy::None;
    return descriptor;
}

} // namespace

class UiTest : public QObject
{
    Q_OBJECT

private slots:
    void busyOverlayFollowsOperation()
    {
        OverlayCoordinator coordinator;
        OperationHandle operation;
        operation.setMessage(QStringLiteral("Calibrating"));
        operation.setCancelable(true);
        operation.setProgress(0.25);

        const QString token = coordinator.bindOperation(&operation);
        QVERIFY(!token.isEmpty());
        QVERIFY(coordinator.busy());
        QCOMPARE(coordinator.busyCount(), 1);
        QCOMPARE(coordinator.busyMessage(), QStringLiteral("Calibrating"));
        QCOMPARE(coordinator.busyProgress(), 0.25);
        QVERIFY(coordinator.busyCancelable());

        operation.setProgress(0.75);
        QCOMPARE(coordinator.busyProgress(), 0.75);

        operation.setState(OperationHandle::State::Succeeded);
        QVERIFY(!coordinator.busy());
    }

    void permissionEvaluatorMapsCoreStatesForQml()
    {
        PermissionEvaluator evaluator;
        const QVariantMap reviewer{{QStringLiteral("role"), QStringLiteral("reviewer")}};
        const QVariantMap operatorContext{{QStringLiteral("role"), QStringLiteral("operator")}};
        const QVariantMap hidden{{QStringLiteral("role"), QStringLiteral("hidden")}};
        const QVariantMap disabled{{QStringLiteral("role"), QStringLiteral("disabled")}};

        QCOMPARE(evaluator.evaluate(QStringLiteral("test.permission"), reviewer),
                 PermissionEvaluator::State::Denied);
        QVERIFY(!evaluator.hasProvider());

        evaluator.setProvider(std::make_shared<TestPermissionProvider>());
        QVERIFY(evaluator.hasProvider());
        QCOMPARE(evaluator.evaluate(QStringLiteral("test.permission"), reviewer),
                 PermissionEvaluator::State::Allow);
        QVERIFY(evaluator.allows(QStringLiteral("test.permission"), reviewer));
        QVERIFY(evaluator.isEditable(QStringLiteral("test.permission"), reviewer));
        QVERIFY(evaluator.isEnabled(QStringLiteral("test.permission"), reviewer));
        QVERIFY(evaluator.isVisible(QStringLiteral("test.permission"), reviewer));

        QCOMPARE(evaluator.evaluate(QStringLiteral("test.permission"), operatorContext),
                 PermissionEvaluator::State::ReadOnly);
        QVERIFY(!evaluator.allows(QStringLiteral("test.permission"), operatorContext));
        QVERIFY(!evaluator.isEditable(QStringLiteral("test.permission"), operatorContext));
        QVERIFY(evaluator.isEnabled(QStringLiteral("test.permission"), operatorContext));
        QVERIFY(evaluator.isVisible(QStringLiteral("test.permission"), operatorContext));

        QVERIFY(!evaluator.isVisible(QStringLiteral("test.permission"), hidden));
        QVERIFY(!evaluator.isEnabled(QStringLiteral("test.permission"), hidden));
        QVERIFY(!evaluator.isEnabled(QStringLiteral("test.permission"), disabled));
        QVERIFY(!evaluator.allows(QStringLiteral("unknown"), reviewer));
    }

    void validationEvaluatorMapsRuleMapsForQml()
    {
        ValidationEvaluator evaluator;
        const QVariantMap rule{
            {QStringLiteral("id"), QStringLiteral("sample.barcode")},
            {QStringLiteral("required"), true},
            {QStringLiteral("minLength"), 4},
            {QStringLiteral("maxLength"), 12},
            {QStringLiteral("regularExpression"), QStringLiteral("[A-Z0-9-]+")}
        };

        const QVariantMap valid = evaluator.validate(QStringLiteral("S-001"), rule);
        QVERIFY(valid.value(QStringLiteral("valid")).toBool());
        QCOMPARE(valid.value(QStringLiteral("ruleId")).toString(), QStringLiteral("sample.barcode"));

        const QVariantMap invalid = evaluator.validate(QStringLiteral("bad value"), rule);
        QVERIFY(!invalid.value(QStringLiteral("valid")).toBool());
        QVERIFY(!invalid.value(QStringLiteral("message")).toString().isEmpty());
        QVERIFY(!evaluator.isValid(QStringLiteral("bad value"), rule));
    }

    void qmlErrorCollectorKeepsStructuredDiagnostics()
    {
        QmlErrorCollector collector;
        QSignalSpy recorded(&collector, &QmlErrorCollector::errorRecorded);

        const QUrl source(QStringLiteral("qrc:/BrokenPage.qml"));
        collector.record(QStringLiteral("/results"), source,
                         QStringLiteral("Type ResultPage unavailable"),
                         QStringLiteral("primary"));

        QCOMPARE(collector.count(), 1);
        QCOMPARE(recorded.count(), 1);
        const auto entry = collector.lastError();
        QCOMPARE(entry.value(QStringLiteral("route")).toString(), QStringLiteral("/results"));
        QCOMPARE(entry.value(QStringLiteral("source")).toUrl(), source);
        QCOMPARE(entry.value(QStringLiteral("stage")).toString(), QStringLiteral("primary"));
        QVERIFY(entry.value(QStringLiteral("message")).toString().contains(QStringLiteral("ResultPage")));

        collector.clear();
        QCOMPARE(collector.count(), 0);
    }

    void quickRuntimeHostSwitchResetsNavigation()
    {
        QQmlEngine engine;
        QQuickItem firstHost;
        QQuickItem secondHost;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));

        runtime.setHost(&firstHost);
        QVERIFY(runtime.push(QStringLiteral("/a")));
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
        QCOMPARE(runtime.depth(), 1);

        runtime.setHost(&secondHost);
        QCOMPARE(runtime.host(), &secondHost);
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.depth(), 0);
        QVERIFY(!runtime.canGoBack());
        QVERIFY(!runtime.canGoForward());
    }

    void quickRuntimeTracksHostDestruction()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));

        auto *host = new QQuickItem;
        runtime.setHost(host);
        QVERIFY(runtime.push(QStringLiteral("/a")));
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
        QCOMPARE(runtime.depth(), 1);

        QSignalSpy changed(&runtime, &QuickRuntime::hostChanged);
        delete host;

        QCOMPARE(runtime.host(), nullptr);
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.depth(), 0);
        QCOMPARE(changed.count(), 1);
    }

    void quickRuntimeRejectsQuickNavigationWithoutHost()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);

        QVERIFY(runtime.registerPage(
            QStringLiteral("/quick"),
            QUrl(QStringLiteral("qrc:/tests/QuickPage.qml")),
            static_cast<int>(PageCachePolicy::KeepAlive),
            false));

        QVERIFY(!runtime.push(QStringLiteral("/quick")));
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.depth(), 0);
        QCOMPARE(runtime.errorCollector()->count(), 1);
        const QVariantMap error = runtime.errorCollector()->lastError();
        QCOMPARE(error.value(QStringLiteral("stage")).toString(), QStringLiteral("host"));
        QVERIFY(error.value(QStringLiteral("message")).toString().contains(QStringLiteral("host"), Qt::CaseInsensitive));
    }

    void quickRuntimeRejectsUnknownRouteAndCanDisableFallback()
    {
        QQmlEngine engine;
        QQuickItem host;
        QuickRuntime runtime(&engine);
        runtime.setHost(&host);

        QVERIFY(runtime.registerPage(
            QStringLiteral("/missing-source"),
            QUrl(QStringLiteral("file:///definitely/not/a/page.qml")),
            static_cast<int>(PageCachePolicy::KeepAlive),
            false));

        QVERIFY(!runtime.push(QStringLiteral("/unknown")));
        QVERIFY(!runtime.push(QStringLiteral("/missing-source")));
        QCOMPARE(runtime.currentRoute(), QString{});
        QCOMPARE(runtime.errorCollector()->count(), 1);
        QCOMPARE(runtime.errorCollector()->lastError().value(QStringLiteral("stage")).toString(),
                 QStringLiteral("primary"));
    }

    void navigationGuardConfirmRetriesExactlyOnce()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        QVERIFY(guard);
        guard->state()->setDirty(true);
        guard->state()->setReason(QStringLiteral("Unsaved result edits"));
        guard->state()->setConfirmRequired(true);

        QSignalSpy confirmation(guard, &NavigationGuardController::confirmationRequested);
        const QVariantMap parameters{{QStringLiteral("sampleId"), QStringLiteral("S-1001")}};
        QVERIFY(!runtime.push(QStringLiteral("/b"), parameters));
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
        QVERIFY(guard->hasPending());
        QCOMPARE(guard->pendingRoute(), QStringLiteral("/b"));
        QCOMPARE(guard->pendingParameters(), parameters);
        QCOMPARE(confirmation.count(), 1);

        QVERIFY(guard->confirmPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/b"));
        QCOMPARE(provider->lastParameters, parameters);
        QVERIFY(!guard->hasPending());
        QCOMPARE(confirmation.count(), 1);
    }

    void navigationGuardCancelKeepsCurrentRoute()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        guard->state()->setDirty(true);
        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QVERIFY(guard->hasPending());

        guard->cancelPending();
        QVERIFY(!guard->hasPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
    }

    void navigationGuardHardBlockCannotBeConfirmed()
    {
        QQmlEngine engine;
        QuickRuntime runtime(&engine);
        auto provider = std::make_shared<TestProvider>();
        QVERIFY(runtime.registerProvider(provider));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/a"))));
        QVERIFY(runtime.registerDescriptor(guardTestPage(QStringLiteral("/b"))));
        QVERIFY(runtime.push(QStringLiteral("/a")));

        auto *guard = runtime.navigationGuard();
        guard->state()->setBusy(true);
        guard->state()->setConfirmRequired(false);
        guard->state()->setReason(QStringLiteral("Calibration is running"));
        QSignalSpy hardBlocked(guard, &NavigationGuardController::hardBlocked);

        QVERIFY(!runtime.push(QStringLiteral("/b")));
        QCOMPARE(hardBlocked.count(), 1);
        QVERIFY(guard->hasPending());
        QVERIFY(!guard->confirmPending());
        QCOMPARE(runtime.currentRoute(), QStringLiteral("/a"));
    }
};

QTEST_MAIN(UiTest)
#include "tst_ui.moc"