
#pragma once

#include <QString>

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
    virtual void destroy() {}
};

} // namespace LfQuick
