#include <LfQuick/Runtime/IPageProvider.h>

#include <LfQuick/Runtime/IPageInstance.h>

namespace LfQuick {

std::unique_ptr<IPageInstance> IPageProvider::createErrorFallback(
    const PageDescriptor &,
    const QVariantMap &,
    const QString &)
{
    return {};
}

} // namespace LfQuick
