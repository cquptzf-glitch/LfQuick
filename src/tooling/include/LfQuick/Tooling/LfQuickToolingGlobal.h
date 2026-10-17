#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKTOOLING_EXPORT
#elif defined(LFQUICKTOOLING_LIBRARY)
#  define LFQUICKTOOLING_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKTOOLING_EXPORT Q_DECL_IMPORT
#endif
