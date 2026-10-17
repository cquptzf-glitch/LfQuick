#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKIO_EXPORT
#elif defined(LFQUICKIO_LIBRARY)
#  define LFQUICKIO_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKIO_EXPORT Q_DECL_IMPORT
#endif
