#pragma once

#include <QtCore/qglobal.h>

#if defined(LFQUICK_STATIC)
#  define LFQUICKLEGACYQCP_EXPORT
#elif defined(LFQUICKLEGACYQCP_LIBRARY)
#  define LFQUICKLEGACYQCP_EXPORT Q_DECL_EXPORT
#else
#  define LFQUICKLEGACYQCP_EXPORT Q_DECL_IMPORT
#endif
