#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKPLATFORM_EXPORT
#elif defined(LFQUICKPLATFORM_LIBRARY)
#  define LFQUICKPLATFORM_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKPLATFORM_EXPORT Q_DECL_IMPORT
#endif
