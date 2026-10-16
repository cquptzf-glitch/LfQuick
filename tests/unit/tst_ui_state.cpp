#include <LfQuick/Core/Permission.h>
#include <LfQuick/Core/Validation.h>
#include <LfQuick/Ui/LfAttached.h>
#include <LfQuick/Ui/PermissionEvaluator.h>
#include <LfQuick/Ui/ValidationFormState.h>
#include <LfQuick/Ui/ValidationRuleTable.h>

#include <QSignalSpy>
#include <QTest>

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
};

QTEST_APPLESS_MAIN(UiStateTest)
#include "tst_ui_state.moc"
