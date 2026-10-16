#include <LfQuick/Core/Route.h>

#include <utility>

namespace LfQuick {

RouteResult::RouteResult() = default;
RouteResult::~RouteResult() = default;
RouteResult::RouteResult(const RouteResult &other) = default;
RouteResult::RouteResult(RouteResult &&other) noexcept = default;
RouteResult &RouteResult::operator=(const RouteResult &other) = default;
RouteResult &RouteResult::operator=(RouteResult &&other) noexcept = default;

RouteResult RouteResult::success(QString routeValue)
{
    RouteResult result;
    result.accepted = true;
    result.route = std::move(routeValue);
    return result;
}

RouteResult RouteResult::failure(QString routeValue, Error routeError)
{
    RouteResult result;
    result.route = std::move(routeValue);
    result.error = std::move(routeError);
    return result;
}

} // namespace LfQuick
