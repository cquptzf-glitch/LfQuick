#pragma once

#include <LfQuick/Runtime/WindowDescriptor.h>

#include <QVariantMap>

namespace LfQuick {

class LFQUICKRUNTIME_EXPORT IWindowInstance
{
public:
    virtual ~IWindowInstance() = default;

    virtual WindowInstanceId instanceId() const = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    virtual bool canClose(QString *reason = nullptr) const
    {
        Q_UNUSED(reason);
        return true;
    }

    virtual bool updateParameters(const QVariantMap &parameters)
    {
        return parameters.isEmpty();
    }

    virtual bool restoreState(const QVariantMap &state)
    {
        return state.isEmpty();
    }

    virtual QVariantMap saveState() const { return {}; }
    virtual void close() = 0;
};

} // namespace LfQuick
