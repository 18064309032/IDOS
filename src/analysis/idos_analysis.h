#ifndef IDOS_ANALYSIS_H
#define IDOS_ANALYSIS_H

#include <qglobal.h>

#if defined(IDOS_ANALYSIS_LIB)
#define ANALYSIS_EXPORT Q_DECL_EXPORT
#else
#define ANALYSIS_EXPORT Q_DECL_IMPORT
#endif

#endif // IDOS_ANALYSIS_H
