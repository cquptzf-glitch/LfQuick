
#pragma once

#include <LfQuick/Runtime/PageDescriptor.h>

#include <memory>

namespace LfQuick {

class IPageInstance;

class IPageProvider
{
public:
    virtual ~IPageProvider() = default;
    virtual QString id() const = 0;
    virtual bool canCreate(const PageDescriptor &descriptor) const = 0;
    virtual std::unique_ptr<IPageInstance> create(const PageDescriptor &descriptor,
                                                   const QVariantMap &parameters) = 0;
};

} // namespace LfQuick
