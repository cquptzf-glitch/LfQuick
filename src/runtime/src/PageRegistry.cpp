#include <LfQuick/Runtime/PageRegistry.h>

#include <utility>

namespace LfQuick {

bool PageRegistry::registerPage(PageDescriptor descriptor)
{
    if (descriptor.route.isEmpty() || descriptor.provider.isEmpty())
        return false;
    if (m_pages.contains(descriptor.route))
        return false;
    m_pages.insert(descriptor.route, std::move(descriptor));
    return true;
}

bool PageRegistry::unregisterPage(QStringView route)
{
    return m_pages.remove(route.toString());
}

std::optional<PageDescriptor> PageRegistry::find(QStringView route) const
{
    const auto it = m_pages.constFind(route.toString());
    if (it == m_pages.cend())
        return std::nullopt;
    return *it;
}

} // namespace LfQuick
