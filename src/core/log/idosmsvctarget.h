#ifndef IDOS_MSVCTARGET_H
#define IDOS_MSVCTARGET_H

#include "idos_core.h"
#include "idoslogtarget.h"

class CORE_EXPORT IDOSMSVCTarget : public IDOSLogTarget
{
  public:
    IDOSMSVCTarget();
    ~IDOSMSVCTarget() override;

    void write(const IDOSLogRecord& record) override;
    void flush() override;
};

#endif // IDOS_MSVCTARGET_H
