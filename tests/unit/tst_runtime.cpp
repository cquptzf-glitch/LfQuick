
#include <LfQuick/Runtime/PageRegistry.h>

#include <QtTest>

class RuntimeTests : public QObject
{
    Q_OBJECT
private slots:
    void registersPage()
    {
        LfQuick::PageRegistry registry;
        LfQuick::PageDescriptor page;
        page.route = "/dashboard";
        page.provider = "quick";
        QVERIFY(registry.registerPage(page));
        QVERIFY(!registry.registerPage(page));
        QCOMPARE(registry.size(), 1);
        QVERIFY(registry.find("/dashboard").has_value());
    }
};

QTEST_APPLESS_MAIN(RuntimeTests)
#include "tst_runtime.moc"
