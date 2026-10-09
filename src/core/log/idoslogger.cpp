#include "idosconsoletarget.h"
#include "idoslogrecord.h"

#include "idoslogger.h"

IDOSLogger::IDOSLogger()
    : QObject(nullptr)
    , m_level(IDOSLogLevel::Trace)
    , m_enabled(true)
    , m_maxRecordCount(5000)
{
    qRegisterMetaType<IDOSLogLevel>();
    qRegisterMetaType<IDOSLogRecord>();
    addTarget(QSharedPointer<IDOSConsoleTarget>::create());
}

IDOSLogger::~IDOSLogger()
{
}

void IDOSLogger::setLevel(IDOSLogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_level = level;
}

IDOSLogLevel IDOSLogger::level() const
{
    QMutexLocker locker(&m_mutex);
    return m_level;
}

void IDOSLogger::addTarget(QSharedPointer<IDOSLogTarget> target)
{
    QMutexLocker locker(&m_mutex);
    m_targets.append(target);
}

void IDOSLogger::removeTarget(QSharedPointer<IDOSLogTarget> target)
{
    QMutexLocker locker(&m_mutex);
    int index = 0;
    while (index < m_targets.count())
    {
        if (m_targets.at(index).data() == target.data())
        {
            m_targets.removeAt(index);
        }
        else
        {
            ++index;
        }
    }
}

void IDOSLogger::clearTargets()
{
    QMutexLocker locker(&m_mutex);
    m_targets.clear();
}

void IDOSLogger::log(IDOSLogLevel level,
                     const QString& message,
                     const QString& file,
                     int line,
                     const QString& function)
{
    QList<QSharedPointer<IDOSLogTarget>> snapshot;
    bool enabled = false;
    IDOSLogLevel currentLevel = IDOSLogLevel::None;

    {
        QMutexLocker locker(&m_mutex);
        enabled = m_enabled;
        currentLevel = m_level;
        snapshot = m_targets;
    }

    if (!enabled || static_cast<int>(level) < static_cast<int>(currentLevel))
    {
        return;
    }

    IDOSLogRecord record(level, message, file, line, function);
    appendRecord(record);
    emit recordAppended(record);

    int index = 0;
    while (index < snapshot.count())
    {
        snapshot.at(index)->write(record);
        ++index;
    }
}

void IDOSLogger::trace(const QString& message,
                       const QString& file,
                       int line,
                       const QString& function)
{
    log(IDOSLogLevel::Trace, message, file, line, function);
}

void IDOSLogger::debug(const QString& message,
                       const QString& file,
                       int line,
                       const QString& function)
{
    log(IDOSLogLevel::Debug, message, file, line, function);
}

void IDOSLogger::info(const QString& message,
                      const QString& file,
                      int line,
                      const QString& function)
{
    log(IDOSLogLevel::Info, message, file, line, function);
}

void IDOSLogger::warn(const QString& message,
                      const QString& file,
                      int line,
                      const QString& function)
{
    log(IDOSLogLevel::Warn, message, file, line, function);
}

void IDOSLogger::error(const QString& message,
                       const QString& file,
                       int line,
                       const QString& function)
{
    log(IDOSLogLevel::Error, message, file, line, function);
}

void IDOSLogger::fatal(const QString& message,
                       const QString& file,
                       int line,
                       const QString& function)
{
    log(IDOSLogLevel::Fatal, message, file, line, function);
}

void IDOSLogger::message(const QString& text, IDOSLogLevel level)
{
    if (text.trimmed().isEmpty())
    {
        return;
    }

    log(level, text);
    emit messagePosted(text, level);
}

void IDOSLogger::enable(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enabled = enabled;
}

bool IDOSLogger::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

QList<IDOSLogRecord> IDOSLogger::records() const
{
    QMutexLocker locker(&m_mutex);
    return m_records;
}

void IDOSLogger::clear()
{
    QMutexLocker locker(&m_mutex);
    m_records.clear();
    emit recordsCleared();
}

IDOSLogger& IDOSLogger::instance()
{
    static IDOSLogger staticInstance;
    return staticInstance;
}

void IDOSLogger::appendRecord(const IDOSLogRecord& record)
{
    QMutexLocker locker(&m_mutex);
    m_records.append(record);
    if (m_records.count() > m_maxRecordCount)
    {
        m_records.removeFirst();
    }
}
