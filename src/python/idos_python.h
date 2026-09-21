#ifndef IDOS_PYTHON_H
#define IDOS_PYTHON_H

#include <qglobal.h>
#if defined(IDOS_PYTHON_LIB)
#define PYTHON_EXPORT Q_DECL_EXPORT
#else
#define PYTHON_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_PYTHON_H
