#ifndef IDOS_GUI_H
#define IDOS_GUI_H

#include <qglobal.h>
#if defined(IDOS_GUI_LIB)
#define GUI_EXPORT Q_DECL_EXPORT
#else
#define GUI_EXPORT Q_DECL_IMPORT
#endif

#endif // !IDOS_GUI_H
