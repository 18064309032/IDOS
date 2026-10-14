#include "idosloglevel.h"

QString idosLogLevelToString(IDOSLogLevel level)
{
    switch (level)
    {
        case IDOSLogLevel::Trace:
            return QStringLiteral("TRACE");
        case IDOSLogLevel::Debug:
            return QStringLiteral("DEBUG");
        case IDOSLogLevel::Info:
            return QStringLiteral("INFO");
        case IDOSLogLevel::Warn:
            return QStringLiteral("WARN");
        case IDOSLogLevel::Error:
            return QStringLiteral("ERROR");
        case IDOSLogLevel::Fatal:
            return QStringLiteral("FATAL");
        case IDOSLogLevel::None:
            return QStringLiteral("NONE");
    }

    return QString();
}

IDOSLogLevel idosLogLevelFromString(const QString& text)
{
    if (text == QStringLiteral("TRACE"))
    {
        return IDOSLogLevel::Trace;
    }
    if (text == QStringLiteral("DEBUG"))
    {
        return IDOSLogLevel::Debug;
    }
    if (text == QStringLiteral("INFO"))
    {
        return IDOSLogLevel::Info;
    }
    if (text == QStringLiteral("WARN"))
    {
        return IDOSLogLevel::Warn;
    }
    if (text == QStringLiteral("ERROR"))
    {
        return IDOSLogLevel::Error;
    }
    if (text == QStringLiteral("FATAL"))
    {
        return IDOSLogLevel::Fatal;
    }

    return IDOSLogLevel::None;
}
