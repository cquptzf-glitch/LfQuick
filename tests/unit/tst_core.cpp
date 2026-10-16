
#include <LfQuick/Core/Error.h>
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
};

QTEST_APPLESS_MAIN(CoreTests)
#include "tst_core.moc"
