#ifndef IDOS_CORE_H
#define IDOS_CORE_H

#include <qglobal.h>
#if defined(IDOS_CORE_LIB)
#define CORE_EXPORT Q_DECL_EXPORT
#else
#define CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_CORE_H
