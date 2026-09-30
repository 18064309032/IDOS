#ifndef IDOS_CONSOLETARGET_H
#define IDOS_CONSOLETARGET_H

#include <QString>

#include "idos_core.h"
#include "idoslogtarget.h"

class CORE_EXPORT IDOSConsoleTarget : public IDOSLogTarget
{
  public:
    IDOSConsoleTarget(bool enableColor = true);
    ~IDOSConsoleTarget() override;

    void write(const IDOSLogRecord& record) override;
    void flush() override;

  private:
    bool m_enableColor;
};

#endif // IDOS_CONSOLETARGET_H
