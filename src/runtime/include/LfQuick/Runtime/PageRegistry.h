
#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>
#include <LfQuick/Runtime/PageDescriptor.h>

#include <QHash>
#include <optional>

namespace LfQuick {

class LFQUICKRUNTIME_EXPORT PageRegistry
{
public:
    bool registerPage(PageDescriptor descriptor);
    bool unregisterPage(QStringView route);
    std::optional<PageDescriptor> find(QStringView route) const;
    qsizetype size() const noexcept { return m_pages.size(); }

private:
    QHash<QString, PageDescriptor> m_pages;
};

} // namespace LfQuick
