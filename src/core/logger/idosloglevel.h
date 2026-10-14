#ifndef IDOS_LOGLEVEL_H
#define IDOS_LOGLEVEL_H

#include <QMetaType>
#include <QString>

#include "idos_core.h"

enum class IDOSLogLevel
{
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    None = 6
};

CORE_EXPORT QString idosLogLevelToString(IDOSLogLevel level);

CORE_EXPORT IDOSLogLevel idosLogLevelFromString(const QString& text);

Q_DECLARE_METATYPE(IDOSLogLevel)

#endif // IDOS_LOGLEVEL_H
