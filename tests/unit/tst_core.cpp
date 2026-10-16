#include <LfQuick/Core/Error.h>
#include <LfQuick/Core/Event.h>
#include <LfQuick/Core/OperationHandle.h>
#include <LfQuick/Core/Permission.h>
#include <LfQuick/Core/Result.h>
#include <LfQuick/Core/Route.h>
#include <LfQuick/Core/Validation.h>

#include <QtTest>

#include <utility>

namespace {

class TestPermissionProvider final : public LfQuick::IPermissionProvider
{
public:
    LfQuick::PermissionState evaluate(QStringView permissionId,
                                      const QVariantMap &) const override
    {
        return permissionId == QStringView(u"core.permission")
            ? LfQuick::PermissionState::Allow
            : LfQuick::PermissionState::Denied;
    }
};

} // namespace

class CoreTests : public QObject
{
    Q_OBJECT
private slots:
    void resultSuccess()
    {
        const auto result = LfQuick::Result<int>::success(42);
        QVERIFY(result);
        QCOMPARE(result.value(), 42);
    }

    void operationClampsProgress()
    {
        LfQuick::OperationHandle operation;
        operation.setProgress(2.0);
        QCOMPARE(operation.progress(), 1.0);
    }

    void eventSubscriptionIsScoped()
    {
        LfQuick::EventDispatcher dispatcher;
        int received = 0;
        {
            auto subscription = dispatcher.subscribe(QStringLiteral("sample.updated"),
                [&](const LfQuick::Event &event) {
                    QCOMPARE(event.payload.toInt(), 7);
                    ++received;
                });
            QCOMPARE(dispatcher.subscriberCount(QStringLiteral("sample.updated")), 1);
            dispatcher.publish({QStringLiteral("sample.updated"), 7});
            QCOMPARE(received, 1);
        }
        QCOMPARE(dispatcher.subscriberCount(QStringLiteral("sample.updated")), 0);
        dispatcher.publish({QStringLiteral("sample.updated"), 7});
        QCOMPARE(received, 1);
    }

    void validationSupportsRequiredLengthAndRegex()
    {
        LfQuick::ValidationRule rule;
        rule.required = true;
        rule.minLength = 4;
        rule.maxLength = 8;
        rule.regularExpression = QStringLiteral("[A-Z0-9-]+");

        QVERIFY(!LfQuick::validateValue(QString(), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("A1"), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("abcd"), rule).valid);
        QVERIFY(LfQuick::validateValue(QStringLiteral("S-001"), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("S-001-XYZ"), rule).valid);
    }

    void validationSupportsRangePrecisionAndOptionalEmpty()
    {
        LfQuick::ValidationRule rule;
        rule.minimum = 1;
        rule.maximum = 100;
        rule.precision = 2;

        QVERIFY(LfQuick::validateValue(QVariant{}, rule).valid);
        QVERIFY(LfQuick::validateValue(QString{}, rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("abc"), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("0.5"), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("100.01"), rule).valid);
        QVERIFY(!LfQuick::validateValue(QStringLiteral("12.345"), rule).valid);
        QVERIFY(LfQuick::validateValue(QStringLiteral("12.34"), rule).valid);
    }

    void permissionProviderSpecialMembersWorkAcrossCoreBoundary()
    {
        TestPermissionProvider provider;
        QCOMPARE(provider.evaluate(QStringView(u"core.permission"), {}),
                 LfQuick::PermissionState::Allow);
        QCOMPARE(provider.evaluate(QStringView(u"unknown"), {}),
                 LfQuick::PermissionState::Denied);
    }

    void routeResultFactoriesAndSpecialMembersWorkAcrossCoreBoundary()
    {
        const auto success = LfQuick::RouteResult::success(QStringLiteral("/results"));
        QVERIFY(success.accepted);
        QCOMPARE(success.route, QStringLiteral("/results"));
        QVERIFY(!success.error.isValid());

        LfQuick::Error error(QStringLiteral("route"), 17, QStringLiteral("blocked"));
        auto failure = LfQuick::RouteResult::failure(QStringLiteral("/review"), error);
        QVERIFY(!failure.accepted);
        QCOMPARE(failure.route, QStringLiteral("/review"));
        QCOMPARE(failure.error.code(), 17);

        LfQuick::RouteResult copied = failure;
        QCOMPARE(copied.error.message(), QStringLiteral("blocked"));
        LfQuick::RouteResult moved = std::move(copied);
        QCOMPARE(moved.route, QStringLiteral("/review"));
    }
};

QTEST_APPLESS_MAIN(CoreTests)
#include "tst_core.moc"
