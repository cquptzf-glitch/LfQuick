#include <LfQuick/Runtime/IPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>

namespace LfQuick {

bool IPageProvider::prewarm(const PageDescriptor &, const QVariantMap &)
{
    return false;
}

void IPageProvider::cancelPrewarm(QStringView)
{
}

PageProviderError IPageProvider::lastErrorInfo() const
{
    const QString message = lastError().trimmed();
    if (message.isEmpty())
        return {};

    PageProviderError error;
    error.code = QStringLiteral("provider-error");
    error.message = message;
    return error;
}

std::unique_ptr<IPageInstance> IPageProvider::createErrorFallback(
    const PageDescriptor &,
    const QVariantMap &,
    const QString &)
{
    return {};
}

} // namespace LfQuick
