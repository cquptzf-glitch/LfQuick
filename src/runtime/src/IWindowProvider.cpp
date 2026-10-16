#include <LfQuick/Runtime/IWindowProvider.h>

namespace LfQuick {

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
