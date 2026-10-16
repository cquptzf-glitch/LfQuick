#pragma once

#include <LfQuick/Core/Error.h>
#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QString>
#include <QVariantMap>

#include <utility>

namespace LfQuick {

struct LFQUICKCORE_EXPORT RouteRequest
{
    QString route;
    QVariantMap parameters;
    QVariantMap metadata;
};

struct LFQUICKCORE_EXPORT RouteResult
{
    bool accepted = false;
    QString route;
    Error error;

    static RouteResult success(QString routeValue)
    {
        RouteResult result;
        result.accepted = true;
        result.route = std::move(routeValue);
        return result;
    }

    static RouteResult failure(QString routeValue, Error routeError)
    {
        RouteResult result;
        result.route = std::move(routeValue);
        result.error = std::move(routeError);
        return result;
    }
};

} // namespace LfQuick
