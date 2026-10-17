#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKSTYLEKIT_EXPORT
#elif defined(LFQUICKSTYLEKIT_LIBRARY)
#  define LFQUICKSTYLEKIT_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKSTYLEKIT_EXPORT Q_DECL_IMPORT
#endif
