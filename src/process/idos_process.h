#ifndef IDOS_PROCESS_H
#define IDOS_PROCESS_H

#include <qglobal.h>
#if defined(IDOS_PROCESS_LIB)
#define PROCESS_EXPORT Q_DECL_EXPORT
#else
#define PROCESS_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_PROCESS_H
