#ifndef IDOS_LOGGER_H
#define IDOS_LOGGER_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>

#include "idos_core.h"
#include "idosloglevel.h"

class IDOSLogRecord;
class IDOSLogTarget;

class CORE_EXPORT IDOSLogger : public QObject
{
    Q_OBJECT

  public:
    IDOSLogger();
    ~IDOSLogger() override;

    void setLevel(IDOSLogLevel level);
    IDOSLogLevel level() const;

    void addTarget(QSharedPointer<IDOSLogTarget> target);
    void removeTarget(QSharedPointer<IDOSLogTarget> target);
    void clearTargets();

    void log(IDOSLogLevel level,
             const QString& message,
             const QString& file = QString(),
             int line = 0,
             const QString& function = QString());

    void trace(const QString& message,
               const QString& file = QString(),
               int line = 0,
               const QString& function = QString());
    void debug(const QString& message,
               const QString& file = QString(),
               int line = 0,
               const QString& function = QString());
    void info(const QString& message,
              const QString& file = QString(),
              int line = 0,
              const QString& function = QString());
    void warn(const QString& message,
              const QString& file = QString(),
              int line = 0,
              const QString& function = QString());
    void error(const QString& message,
               const QString& file = QString(),
               int line = 0,
               const QString& function = QString());
    void fatal(const QString& message,
               const QString& file = QString(),
               int line = 0,
               const QString& function = QString());

    void message(const QString& text, IDOSLogLevel level = IDOSLogLevel::Info);

    void enable(bool enabled);
    bool isEnabled() const;

    QList<IDOSLogRecord> records() const;
    void clear();

    static IDOSLogger& instance();

  signals:
    void recordAppended(const IDOSLogRecord& record);
    void messagePosted(const QString& message, IDOSLogLevel level);
    void recordsCleared();

  private:
    void appendRecord(const IDOSLogRecord& record);

    IDOSLogLevel m_level;
    mutable QMutex m_mutex;
    bool m_enabled;
    QList<QSharedPointer<IDOSLogTarget>> m_targets;
    QList<IDOSLogRecord> m_records;
    int m_maxRecordCount;

    IDOSLogger(IDOSLogger&&) = delete;
    IDOSLogger(const IDOSLogger&) = delete;
    IDOSLogger& operator=(IDOSLogger&&) = delete;
    IDOSLogger& operator=(const IDOSLogger&) = delete;
};

#define IDOS_TRACE(msg)\
    IDOSLogger::instance().trace(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_DEBUG(msg)\
    IDOSLogger::instance().debug(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_INFO(msg)\
    IDOSLogger::instance().info(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_WARN(msg)\
    IDOSLogger::instance().warn(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_ERROR(msg)\
    IDOSLogger::instance().error(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_FATAL(msg)\
    IDOSLogger::instance().fatal(msg, __FILE__, __LINE__, Q_FUNC_INFO)

#define IDOS_MESSAGE(msg, level)\
    IDOSLogger::instance().message(msg, level)

#define IDOS_TRACE_IF(cond, msg)\
    do { if (cond) { IDOS_TRACE(msg); } } while (false)

#define IDOS_DEBUG_IF(cond, msg)\
    do { if (cond) { IDOS_DEBUG(msg); } } while (false)

#define IDOS_INFO_IF(cond, msg)\
    do { if (cond) { IDOS_INFO(msg); } } while (false)

#define IDOS_WARN_IF(cond, msg)\
    do { if (cond) { IDOS_WARN(msg); } } while (false)

#define IDOS_ERROR_IF(cond, msg)\
    do { if (cond) { IDOS_ERROR(msg); } } while (false)

#define IDOS_FATAL_IF(cond, msg)\
    do { if (cond) { IDOS_FATAL(msg); } } while (false)

#define IDOS_MESSAGE_IF(cond, msg, level)\
    do { if (cond) { IDOS_MESSAGE(msg, level); } } while (false)

#endif // IDOS_LOGGER_H
