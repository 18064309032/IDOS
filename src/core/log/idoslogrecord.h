#ifndef IDOS_LOGRECORD_H
#define IDOS_LOGRECORD_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

#include "idos_core.h"
#include "idosloglevel.h"

class CORE_EXPORT IDOSLogRecord
{
  public:
    IDOSLogRecord();
    IDOSLogRecord(IDOSLogLevel level,
                  const QString& message,
                  const QString& file,
                  int line,
                  const QString& function);

    IDOSLogLevel level() const;
    QString message() const;
    QDateTime time() const;
    QString file() const;
    int line() const;
    QString function() const;

  private:
    IDOSLogLevel m_level;
    QString m_message;
    QDateTime m_time;
    QString m_file;
    int m_line;
    QString m_function;
};

Q_DECLARE_METATYPE(IDOSLogRecord)

#endif // IDOS_LOGRECORD_H
