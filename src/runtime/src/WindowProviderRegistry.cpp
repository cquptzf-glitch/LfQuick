#include <LfQuick/Runtime/WindowProviderRegistry.h>
#include <LfQuick/Runtime/IWindowProvider.h>

namespace LfQuick {

bool WindowProviderRegistry::registerProvider(std::shared_ptr<IWindowProvider> provider)
{
    if (!provider)
        return false;

    const auto providerId = provider->id();
    if (providerId.isEmpty() || m_providers.contains(providerId))
        return false;

    m_providers.insert(providerId, std::move(provider));
    return true;
}

bool WindowProviderRegistry::unregisterProvider(QStringView id)
{
    return m_providers.remove(id.toString()) > 0;
}

std::shared_ptr<IWindowProvider> WindowProviderRegistry::find(QStringView id) const
{
    const auto it = m_providers.constFind(id.toString());
    return it == m_providers.cend() ? nullptr : it.value();
}

} // namespace LfQuick
