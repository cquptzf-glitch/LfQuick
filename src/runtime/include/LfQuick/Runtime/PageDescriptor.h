
#pragma once

#include <QString>
#include <QVariantMap>

namespace LfQuick {

enum class PageCachePolicy { None, Singleton, KeepAlive, Lru };
enum class FallbackPolicy { None, ErrorPage, LegacyBeforeActivate, LegacyIfStateless };

struct PageDescriptor
{
    QString route;
    QString provider;
    QVariantMap source;
    PageCachePolicy cachePolicy = PageCachePolicy::None;
    FallbackPolicy fallbackPolicy = FallbackPolicy::ErrorPage;
    QVariantMap metadata;
};

} // namespace LfQuick
