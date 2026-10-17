#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QVariantMap>

namespace LfQuick {

// Technology-neutral state contract shared by page and window instances.
// Empty state means the instance has nothing durable to preserve.
class LFQUICKRUNTIME_EXPORT IStatefulInstance
{
public:
    virtual ~IStatefulInstance() = default;

    virtual QVariantMap saveState() const { return {}; }
    virtual bool restoreState(const QVariantMap &state)
    {
        return state.isEmpty();
    }
};

} // namespace LfQuick
