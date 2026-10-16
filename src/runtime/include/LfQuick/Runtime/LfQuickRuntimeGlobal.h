
#pragma once
#include <QtCore/qglobal.h>
#if defined(LFQUICKRUNTIME_LIBRARY)
#  define LFQUICKRUNTIME_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKRUNTIME_EXPORT Q_DECL_IMPORT
#endif
