#include <LfQuick/Runtime/PageProviderRegistry.h>
#include <LfQuick/Runtime/IPageProvider.h>

namespace LfQuick {

bool PageProviderRegistry::registerProvider(std::shared_ptr<IPageProvider> provider)
{
    if (!provider)
        return false;
    const auto providerId = provider->id();
    if (providerId.isEmpty() || m_providers.contains(providerId))
        return false;
    m_providers.insert(providerId, std::move(provider));
    return true;
}

bool PageProviderRegistry::unregisterProvider(QStringView id)
{
    return m_providers.remove(id.toString());
}

std::shared_ptr<IPageProvider> PageProviderRegistry::find(QStringView id) const
{
    const auto it = m_providers.constFind(id.toString());
    return it == m_providers.cend() ? nullptr : it.value();
}

} // namespace LfQuick
