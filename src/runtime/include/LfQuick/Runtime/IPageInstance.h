#pragma once

#include <QString>
#include <QVariantMap>

namespace LfQuick {

class IPageInstance
{
public:
    virtual ~IPageInstance() = default;
    virtual QString pageId() const = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void suspend() = 0;
    virtual void resume() = 0;

    // Cache policies such as Singleton can reuse a live page instance for a
    // later navigation request. Implementations that accept navigation
    // parameters should refresh their exposed state here. Returning false
    // rejects reuse instead of silently showing stale parameters.
    virtual bool updateParameters(const QVariantMap &parameters)
    {
        return parameters.isEmpty();
    }

    virtual void destroy() {}
};

} // namespace LfQuick
