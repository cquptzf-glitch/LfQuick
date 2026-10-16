#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKUI_EXPORT
#elif defined(LFQUICKUI_LIBRARY)
#  define LFQUICKUI_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKUI_EXPORT Q_DECL_IMPORT
#endif
