#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKRUNTIME_EXPORT
#elif defined(LFQUICKRUNTIME_LIBRARY)
#  define LFQUICKRUNTIME_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKRUNTIME_EXPORT Q_DECL_IMPORT
#endif
