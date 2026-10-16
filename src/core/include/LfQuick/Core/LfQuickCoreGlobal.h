#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKCORE_EXPORT
#elif defined(LFQUICKCORE_LIBRARY)
#  define LFQUICKCORE_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKCORE_EXPORT Q_DECL_IMPORT
#endif
