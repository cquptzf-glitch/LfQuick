#pragma once

#include <LfQuick/Core/Error.h>
#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QString>
#include <QVariantMap>

namespace LfQuick {

struct LFQUICKCORE_EXPORT RouteRequest
{
    QString route;
    QVariantMap parameters;
    QVariantMap metadata;
};

struct LFQUICKCORE_EXPORT RouteResult
{
    RouteResult();
    ~RouteResult();
    RouteResult(const RouteResult &other);
    RouteResult(RouteResult &&other) noexcept;
    RouteResult &operator=(const RouteResult &other);
    RouteResult &operator=(RouteResult &&other) noexcept;

    bool accepted = false;
    QString route;
    Error error;

    static RouteResult success(QString routeValue);
    static RouteResult failure(QString routeValue, Error routeError);
};

} // namespace LfQuick
