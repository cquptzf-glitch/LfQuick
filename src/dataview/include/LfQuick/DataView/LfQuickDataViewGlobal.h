#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKDATAVIEW_EXPORT
#elif defined(LFQUICKDATAVIEW_LIBRARY)
#  define LFQUICKDATAVIEW_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKDATAVIEW_EXPORT Q_DECL_IMPORT
#endif
