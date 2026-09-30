#include "idoslogrecord.h"

IDOSLogRecord::IDOSLogRecord()
    : m_level(IDOSLogLevel::Debug)
    , m_time(QDateTime::currentDateTime())
    , m_line(0)
{
}

IDOSLogRecord::IDOSLogRecord(IDOSLogLevel level,
                             const QString& message,
                             const QString& file,
                             int line,
                             const QString& function)
    : m_level(level)
    , m_message(message)
    , m_time(QDateTime::currentDateTime())
    , m_file(file)
    , m_line(line)
    , m_function(function)
{
}

IDOSLogLevel IDOSLogRecord::level() const
{
    return m_level;
}

QString IDOSLogRecord::message() const
{
    return m_message;
}

QDateTime IDOSLogRecord::time() const
{
    return m_time;
}

QString IDOSLogRecord::file() const
{
    return m_file;
}

int IDOSLogRecord::line() const
{
    return m_line;
}

QString IDOSLogRecord::function() const
{
    return m_function;
}
