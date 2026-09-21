#ifndef IDOS_PROVIDERS_H
#define IDOS_PROVIDERS_H

#include <qglobal.h>
#if defined(IDOS_PROVIDERS_LIB)
#define PROVIDERS_EXPORT Q_DECL_EXPORT
#else
#define PROVIDERS_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_PROVIDERS_H
