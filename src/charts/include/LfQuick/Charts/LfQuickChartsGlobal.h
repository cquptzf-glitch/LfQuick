#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKCHARTS_EXPORT
#elif defined(LFQUICKCHARTS_LIBRARY)
#  define LFQUICKCHARTS_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKCHARTS_EXPORT Q_DECL_IMPORT
#endif
