#include <LfQuick/Core/Permission.h>
#include <LfQuick/Core/Validation.h>
#include <LfQuick/Ui/HelpContextRegistry.h>
#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/LfValidationBinding.h>
#include <LfQuick/Ui/OverlayBackend.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/PermissionEvaluator.h>
#include <LfQuick/Ui/ValidationFormState.h>
#include <LfQuick/Ui/ValidationRuleTable.h>

#include <QSignalSpy>
#include <QTest>
#include <QtQml/qqml.h>

#include <memory>

using namespace LfQuick;

namespace {

class TestPermissionProvider final : public IPermissionProvider
{
public:
    PermissionState evaluate(QStringView permissionId,
                             const QVariantMap &context) const override
    {
        if (permissionId != QStringView(u"test.review"))
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

class TestValidationProvider final : public IValidationRuleProvider
{
public:
    ValidationRule rule(QStringView id) const override
    {
        if (id != QStringView(u"provider.code"))
            return {};

        ValidationRule result;
        result.id = QStringLiteral("provider.code");
        result.required = true;
        result.minLength = 2;
        result.maxLength = 4;
        return result;
    }
};

class TestOverlayBackend final : public IOverlayBackend
{
public:
    using IOverlayBackend::IOverlayBackend;

    void presentToast(const ToastRequest &request) override
    {
        presentedToasts.append(request);
    }

    void dismissToast(const QString &requestId) override
    {
        dismissedToasts.append(requestId);
    }

    void presentDialog(const DialogRequest &request) override
    {
        presentedDialogs.append(request);
    }

    void dismissDialog(const QString &requestId) override
    {
        dismissedDialogs.append(requestId);
    }

    void simulateToastDismiss(const QString &requestId)
    {
        emit toastDismissRequested(requestId);
    }

    void simulateDialogResult(const QString &requestId, const QString &buttonId)
    {
        emit dialogResolved(requestId, buttonId);
    }

    QList<ToastRequest> presentedToasts;
    QStringList dismissedToasts;
    QList<DialogRequest> presentedDialogs;
    QStringList dismissedDialogs;
};

bool containsToast(const QVariantList &toasts, const QString &requestId)
{
    for (const QVariant &toast : toasts) {
        if (toast.toMap().value(QStringLiteral("id")).toString() == requestId)
            return true;
    }
    return false;
}

} // namespace

class UiStateTest : public QObject
{
    Q_OBJECT

private slots:
    void attachedPermissionIsOptInAndFailClosedWhenConfigured()
    {
        QObject target;
        LfAttached attached(&target);
        PermissionBinding *permission = attached.permission();
        QVERIFY(permission);

        QVERIFY(!permission->active());
        QCOMPARE(permission->state(), PermissionEvaluator::State::Allow);
        QVERIFY(permission->allowed());
        QVERIFY(permission->visible());
        QVERIFY(permission->enabled());
        QVERIFY(permission->editable());

        permission->setPermissionId(QStringLiteral("test.review"));
        QVERIFY(permission->active());
        QCOMPARE(permission->state(), PermissionEvaluator::State::Denied);
        QVERIFY(permission->visible());
        QVERIFY(!permission->enabled());
        QVERIFY(!permission->editable());

        PermissionEvaluator evaluator;
        evaluator.setProvider(std::make_shared<TestPermissionProvider>());
        permission->setEvaluator(&evaluator);
        permission->setContext({{QStringLiteral("role"), QStringLiteral("reviewer")}});
        QCOMPARE(permission->state(), PermissionEvaluator::State::Allow);
        QVERIFY(permission->allowed());
        QVERIFY(permission->editable());

        permission->setContext({{QStringLiteral("role"), QStringLiteral("operator")}});
        QCOMPARE(permission->state(), PermissionEvaluator::State::ReadOnly);
        QVERIFY(!permission->allowed());
        QVERIFY(permission->visible());
        QVERIFY(permission->enabled());
        QVERIFY(!permission->editable());

        permission->setContext({{QStringLiteral("role"), QStringLiteral("hidden")}});
        QCOMPARE(permission->state(), PermissionEvaluator::State::Hidden);
        QVERIFY(!permission->visible());
        QVERIFY(!permission->enabled());
    }

    void attachedPermissionReactsToProviderChanges()
    {
        QObject target;
        LfAttached attached(&target);
        PermissionBinding *permission = attached.permission();
        PermissionEvaluator evaluator;
        permission->setEvaluator(&evaluator);
        permission->setPermissionId(QStringLiteral("test.review"));
        permission->setContext({{QStringLiteral("role"), QStringLiteral("reviewer")}});

        QSignalSpy changed(permission, &PermissionBinding::stateChanged);
        QCOMPARE(permission->state(), PermissionEvaluator::State::Denied);

        evaluator.setProvider(std::make_shared<TestPermissionProvider>());
        QVERIFY(changed.count() >= 1);
        QCOMPARE(permission->state(), PermissionEvaluator::State::Allow);
    }

    void attachedValidationBindingTracksRuleAndTargetLifetime()
    {
        auto *target = new QObject;
        auto *attached = qmlAttachedPropertiesObject<Lf>(target, true);
        QVERIFY(attached);
        attached->setValidationRule({
            {QStringLiteral("id"), QStringLiteral("sample.code")},
            {QStringLiteral("required"), true},
            {QStringLiteral("minLength"), 2},
            {QStringLiteral("maxLength"), 4}
        });

        LfValidationBinding binding;
        binding.setTarget(target);
        binding.setValue(QStringLiteral("A"));
        QVERIFY(!binding.valid());
        QCOMPARE(binding.rule().value(QStringLiteral("id")).toString(), QStringLiteral("sample.code"));

        binding.setValue(QStringLiteral("AB"));
        QVERIFY(binding.valid());

        attached->setValidationRule({
            {QStringLiteral("id"), QStringLiteral("sample.code")},
            {QStringLiteral("required"), true},
            {QStringLiteral("maxLength"), 1}
        });
        QVERIFY(!binding.valid());
        QCOMPARE(binding.rule().value(QStringLiteral("maxLength")).toInt(), 1);

        QSignalSpy targetChanged(&binding, &LfValidationBinding::targetChanged);
        delete target;

        QCOMPARE(binding.target(), nullptr);
        QVERIFY(binding.rule().isEmpty());
        QVERIFY(binding.valid());
        QCOMPARE(targetChanged.count(), 1);
    }

    void helpRegistryResolvesAttachedContext()
    {
        QObject target;
        auto *attached = qmlAttachedPropertiesObject<Lf>(&target, true);
        QVERIFY(attached);
        attached->setHelpContext(QStringLiteral("sample.register"));

        HelpContextRegistry registry;
        QVERIFY(registry.registerContext(
            QStringLiteral("sample.register"),
            {{QStringLiteral("title"), QStringLiteral("Register sample")},
             {QStringLiteral("text"), QStringLiteral("Synthetic help text")},
             {QStringLiteral("url"), QStringLiteral("help://sample/register")}}));

        QCOMPARE(registry.contextIdFor(&target), QStringLiteral("sample.register"));
        QCOMPARE(registry.resolveFor(&target).value(QStringLiteral("title")).toString(),
                 QStringLiteral("Register sample"));

        QSignalSpy requested(&registry, &HelpContextRegistry::helpRequested);
        QVERIFY(registry.requestHelpFor(&target));
        QCOMPARE(requested.count(), 1);
        QCOMPARE(requested.at(0).at(0).toString(), QStringLiteral("sample.register"));

        attached->setHelpContext(QStringLiteral("missing"));
        QVERIFY(!registry.requestHelpFor(&target));
        QCOMPARE(requested.count(), 1);
    }

    void ruleTableSupportsLocalRulesAndProviderFallback()
    {
        ValidationRuleTable table;
        table.setRules({
            {QStringLiteral("sample.id"), QVariantMap{
                {QStringLiteral("required"), true},
                {QStringLiteral("minLength"), 3},
                {QStringLiteral("maxLength"), 8}
            }}
        });

        QCOMPARE(table.count(), 1);
        QVERIFY(table.contains(QStringLiteral("sample.id")));
        QVERIFY(table.isValid(QStringLiteral("sample.id"), QStringLiteral("S01")));
        QVERIFY(!table.isValid(QStringLiteral("sample.id"), QStringLiteral("S")));

        const QVariantMap missing = table.validate(QStringLiteral("missing"), QStringLiteral("x"));
        QVERIFY(!missing.value(QStringLiteral("valid")).toBool());
        QVERIFY(missing.value(QStringLiteral("message")).toString().contains(QStringLiteral("missing")));

        table.setProvider(std::make_shared<TestValidationProvider>());
        QVERIFY(table.hasProvider());
        QVERIFY(table.contains(QStringLiteral("provider.code")));
        QVERIFY(table.isValid(QStringLiteral("provider.code"), QStringLiteral("AB")));
        QVERIFY(!table.isValid(QStringLiteral("provider.code"), QStringLiteral("A")));
    }

    void formStateAggregatesRuleAndBusinessErrors()
    {
        ValidationRuleTable table;
        table.setRules({
            {QStringLiteral("sample.id"), QVariantMap{
                {QStringLiteral("required"), true},
                {QStringLiteral("minLength"), 3},
                {QStringLiteral("maxLength"), 8}
            }},
            {QStringLiteral("sample.rack"), QVariantMap{
                {QStringLiteral("required"), true},
                {QStringLiteral("minLength"), 1},
                {QStringLiteral("maxLength"), 4}
            }}
        });

        ValidationFormState form;
        form.setRuleTable(&table);
        ValidationFieldState *sampleId = form.field(QStringLiteral("sampleId"),
                                                     QStringLiteral("sample.id"));
        ValidationFieldState *rack = form.field(QStringLiteral("rack"),
                                                 QStringLiteral("sample.rack"));
        QVERIFY(sampleId);
        QVERIFY(rack);
        QCOMPARE(form.fieldCount(), 2);
        QVERIFY(form.valid());
        QVERIFY(!sampleId->touched());

        QVERIFY(!form.validateField(QStringLiteral("sampleId"), QStringLiteral("S")));
        QVERIFY(sampleId->touched());
        QVERIFY(sampleId->hasError());
        QCOMPARE(form.errorCount(), 1);

        QVERIFY(form.validateField(QStringLiteral("sampleId"), QStringLiteral("S01")));
        QVERIFY(form.validateField(QStringLiteral("rack"), QStringLiteral("R1")));
        QVERIFY(form.valid());

        form.setExternalError(QStringLiteral("sampleId"), QStringLiteral("Sample ID already exists"));
        QVERIFY(!form.valid());
        QVERIFY(sampleId->hasError());
        QCOMPARE(sampleId->message(), QStringLiteral("Sample ID already exists"));

        form.clearExternalError(QStringLiteral("sampleId"));
        QVERIFY(form.valid());
        QVERIFY(sampleId->message().isEmpty());

        QVERIFY(table.setRule(QStringLiteral("sample.id"), {
            {QStringLiteral("required"), true},
            {QStringLiteral("maxLength"), 2}
        }));
        QVERIFY(!form.valid());
        QVERIFY(sampleId->hasError());

        form.reset();
        QVERIFY(form.valid());
        QVERIFY(!sampleId->touched());
        QVERIFY(!rack->touched());
    }

    void toastCoordinatorDeduplicatesAndEnforcesLimit()
    {
        OverlayCoordinator coordinator;
        coordinator.setMaxVisibleToasts(2);

        const QString first = coordinator.showToast(
            QStringLiteral("Connected"),
            {{QStringLiteral("severity"), QStringLiteral("success")},
             {QStringLiteral("durationMs"), 0},
             {QStringLiteral("dedupKey"), QStringLiteral("connection")}});
        QVERIFY(!first.isEmpty());
        QCOMPARE(coordinator.toastCount(), 1);

        const QString duplicate = coordinator.showToast(
            QStringLiteral("Connected again"),
            {{QStringLiteral("severity"), QStringLiteral("success")},
             {QStringLiteral("durationMs"), 0},
             {QStringLiteral("dedupKey"), QStringLiteral("connection")}});
        QCOMPARE(duplicate, first);
        QCOMPARE(coordinator.toastCount(), 1);
        QCOMPARE(coordinator.toasts().constLast().toMap().value(QStringLiteral("message")).toString(),
                 QStringLiteral("Connected again"));

        const QString second = coordinator.showToast(
            QStringLiteral("Queued"), {{QStringLiteral("durationMs"), 0}});
        const QString third = coordinator.showToast(
            QStringLiteral("Completed"), {{QStringLiteral("durationMs"), 0}});
        QVERIFY(!second.isEmpty());
        QVERIFY(!third.isEmpty());
        QCOMPARE(coordinator.toastCount(), 2);
        QVERIFY(!containsToast(coordinator.toasts(), first));
        QVERIFY(containsToast(coordinator.toasts(), second));
        QVERIFY(containsToast(coordinator.toasts(), third));

        TestOverlayBackend backend;
        coordinator.setBackend(&backend);
        QVERIFY(coordinator.externalBackendActive());
        QCOMPARE(backend.presentedToasts.size(), 2);

        backend.simulateToastDismiss(second);
        QCOMPARE(coordinator.toastCount(), 1);
        QVERIFY(backend.dismissedToasts.contains(second));
    }

    void dialogCoordinatorSerializesRequestsAcrossBackend()
    {
        OverlayCoordinator coordinator;
        TestOverlayBackend backend;
        coordinator.setBackend(&backend);
        QSignalSpy resolved(&coordinator, &OverlayCoordinator::dialogResolved);

        const QString first = coordinator.requestDialog(
            QStringLiteral("Leave page?"), QStringLiteral("Unsaved changes"),
            {{QStringLiteral("buttons"), QStringList{QStringLiteral("yes"), QStringLiteral("no")}},
             {QStringLiteral("defaultButton"), QStringLiteral("no")},
             {QStringLiteral("context"), QVariantMap{{QStringLiteral("kind"), QStringLiteral("guard")}}}});
        const QString second = coordinator.requestDialog(
            QStringLiteral("Notice"), QStringLiteral("Queued dialog"),
            {{QStringLiteral("buttons"), QStringList{QStringLiteral("ok")}}});

        QVERIFY(!first.isEmpty());
        QVERIFY(!second.isEmpty());
        QVERIFY(coordinator.dialogActive());
        QCOMPARE(coordinator.pendingDialogCount(), 2);
        QCOMPARE(coordinator.activeDialog().value(QStringLiteral("id")).toString(), first);
        QCOMPARE(backend.presentedDialogs.size(), 1);

        QVERIFY(!coordinator.resolveDialog(first, QStringLiteral("unsupported")));
        backend.simulateDialogResult(first, QStringLiteral("yes"));
        QCOMPARE(resolved.count(), 1);
        QCOMPARE(resolved.at(0).at(0).toString(), first);
        QCOMPARE(resolved.at(0).at(1).toString(), QStringLiteral("yes"));
        QCOMPARE(resolved.at(0).at(2).toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("guard"));

        QCOMPARE(coordinator.activeDialog().value(QStringLiteral("id")).toString(), second);
        QCOMPARE(backend.presentedDialogs.size(), 2);
        QVERIFY(coordinator.cancelDialog(second));
        QCOMPARE(resolved.count(), 2);
        QCOMPARE(resolved.at(1).at(1).toString(), QStringLiteral("ok"));
        QVERIFY(!coordinator.dialogActive());
        QCOMPARE(coordinator.pendingDialogCount(), 0);
    }
};

QTEST_APPLESS_MAIN(UiStateTest)
#include "tst_ui_state.moc"
