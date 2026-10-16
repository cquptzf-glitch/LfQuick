#include <LfQuick/Runtime/IPageInstance.h>
#include <LfQuick/Runtime/IPageProvider.h>

#include <QtTest>

class RuntimeAbiProvider final : public LfQuick::IPageProvider
{
public:
    QString id() const override { return QStringLiteral("abi-test"); }
    bool canCreate(const LfQuick::PageDescriptor &) const override { return true; }
    std::unique_ptr<LfQuick::IPageInstance> create(
        const LfQuick::PageDescriptor &, const QVariantMap &) override
    {
        return {};
    }
};

class RuntimeAbiTests : public QObject
{
    Q_OBJECT
private slots:
    void defaultErrorFallbackCrossesDllBoundary()
    {
        RuntimeAbiProvider provider;
        LfQuick::PageDescriptor descriptor;
        descriptor.route = QStringLiteral("/abi-test");
        descriptor.provider = provider.id();

        const auto fallback = provider.createErrorFallback(
            descriptor, {}, QStringLiteral("primary failure"));
        QVERIFY(!fallback);
    }
};

QTEST_APPLESS_MAIN(RuntimeAbiTests)
#include "tst_runtime_abi.moc"
