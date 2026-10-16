#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QHash>
#include <QString>
#include <QStringView>

#include <memory>

namespace LfQuick {

class IWindowProvider;

class LFQUICKRUNTIME_EXPORT WindowProviderRegistry
{
public:
    bool registerProvider(std::shared_ptr<IWindowProvider> provider);
    bool unregisterProvider(QStringView id);
    std::shared_ptr<IWindowProvider> find(QStringView id) const;
    qsizetype size() const noexcept { return m_providers.size(); }

private:
    QHash<QString, std::shared_ptr<IWindowProvider>> m_providers;
};

} // namespace LfQuick
