#include <LfQuick/Core/Error.h>
#include <LfQuick/Core/Event.h>
#include <LfQuick/Core/OperationHandle.h>
#include <LfQuick/Core/Result.h>

#include <QtTest>

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
};

QTEST_APPLESS_MAIN(CoreTests)
#include "tst_core.moc"
