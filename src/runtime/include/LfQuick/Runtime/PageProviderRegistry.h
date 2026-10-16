#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QHash>
#include <QString>
#include <QStringView>

#include <memory>

namespace LfQuick {

class IPageProvider;

class LFQUICKRUNTIME_EXPORT PageProviderRegistry
{
public:
    bool registerProvider(std::shared_ptr<IPageProvider> provider);
    bool unregisterProvider(QStringView id);
    std::shared_ptr<IPageProvider> find(QStringView id) const;
    qsizetype size() const noexcept { return m_providers.size(); }

private:
    QHash<QString, std::shared_ptr<IPageProvider>> m_providers;
};

} // namespace LfQuick
