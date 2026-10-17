#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKMODELS_EXPORT
#elif defined(LFQUICKMODELS_LIBRARY)
#  define LFQUICKMODELS_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKMODELS_EXPORT Q_DECL_IMPORT
#endif
