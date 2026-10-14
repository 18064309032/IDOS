#ifndef IDOS_LOGTARGET_H
#define IDOS_LOGTARGET_H

#include "idos_core.h"

class IDOSLogRecord;

class CORE_EXPORT IDOSLogTarget
{
  public:
    virtual ~IDOSLogTarget()
    {
    }

    virtual void write(const IDOSLogRecord& record) = 0;
    virtual void flush() = 0;
};

#endif // IDOS_LOGTARGET_H
