#pragma once

#include <QString>
#include <QVariantMap>

#include <optional>

namespace LfQuick {

enum class PageCachePolicy { None, Singleton, KeepAlive, Lru };
enum class FallbackPolicy { None, ErrorPage, LegacyBeforeActivate, LegacyIfStateless };

struct PageFallbackDescriptor
{
    QString provider;
    QVariantMap source;
};

struct PageDescriptor
{
    QString route;
    QString provider;
    QVariantMap source;
    PageCachePolicy cachePolicy = PageCachePolicy::None;
    FallbackPolicy fallbackPolicy = FallbackPolicy::ErrorPage;
    std::optional<PageFallbackDescriptor> fallback;
    QVariantMap metadata;
};

} // namespace LfQuick
