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

    // Returns whether the provider-specific backing object is still usable.
    // The default keeps existing providers source-compatible. Providers that
    // wrap externally owned UI objects should override this so Runtime can
    // recreate cached entries after those backing objects are destroyed.
    virtual bool isValid() const noexcept { return true; }

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
