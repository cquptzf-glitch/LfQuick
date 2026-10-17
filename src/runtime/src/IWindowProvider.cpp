#include <LfQuick/Runtime/IWindowProvider.h>

#include <LfQuick/Runtime/IWindowBacking.h>
#include <LfQuick/Runtime/IWindowInstance.h>

namespace LfQuick {

bool IWindowProvider::canAdoptBacking(const WindowDescriptor &descriptor,
                                      const IWindowBacking &backing) const
{
    Q_UNUSED(descriptor);
    Q_UNUSED(backing);
    return false;
}

std::unique_ptr<IWindowInstance> IWindowProvider::createWithBacking(
    const WindowDescriptor &descriptor,
    const WindowOpenRequest &request,
    std::unique_ptr<IWindowBacking> &backing)
{
    Q_UNUSED(descriptor);
    Q_UNUSED(request);
    Q_UNUSED(backing);
    return {};
}

WindowProviderError IWindowProvider::lastErrorInfo() const
{
    const QString message = lastError().trimmed();
    if (message.isEmpty())
        return {};

    WindowProviderError error;
    error.code = QStringLiteral("provider-error");
    error.message = message;
    return error;
}

} // namespace LfQuick
