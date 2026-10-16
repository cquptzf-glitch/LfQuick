
#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICKCORE_LIBRARY)
#  define LFQUICKCORE_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKCORE_EXPORT Q_DECL_IMPORT
#endif
