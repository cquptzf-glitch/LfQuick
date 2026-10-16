#pragma once

#include <LfQuick/Runtime/LfQuickRuntimeGlobal.h>

#include <QString>
#include <QVariantMap>

#include <utility>

namespace LfQuick {

enum class NavigationAction {
    Push,
    Replace,
    Back
};

struct NavigationContext
{
    NavigationAction action = NavigationAction::Push;
    QString fromRoute;
    QString toRoute;
    QVariantMap parameters;
};

struct NavigationDecision
{
    bool allowed = true;
    QString reason;

    static NavigationDecision allow()
    {
        return {};
    }

    static NavigationDecision deny(QString reasonValue)
    {
        NavigationDecision decision;
        decision.allowed = false;
        decision.reason = std::move(reasonValue);
        return decision;
    }
};

class LFQUICKRUNTIME_EXPORT INavigationGuard
{
public:
    virtual ~INavigationGuard() = default;
    virtual NavigationDecision evaluate(const NavigationContext &context) = 0;
};

} // namespace LfQuick
