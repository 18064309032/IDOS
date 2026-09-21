#ifndef IDOS_APP_H
#define IDOS_APP_H

#include <qglobal.h>
#if defined(IDOS_APP_LIB)
#define APP_EXPORT Q_DECL_EXPORT
#else
#define APP_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_APP_H
