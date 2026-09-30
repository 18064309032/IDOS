#include "idosmsvctarget.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "idosloglevel.h"
#include "idoslogrecord.h"

IDOSMSVCTarget::IDOSMSVCTarget()
{
}

IDOSMSVCTarget::~IDOSMSVCTarget()
{
}

void IDOSMSVCTarget::write(const IDOSLogRecord& record)
{
    QString formatted =
        QStringLiteral("[%1] [%2] %3")
            .arg(record.time().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                 idosLogLevelToString(record.level()),
                 record.message());
#ifdef Q_OS_WIN
    QByteArray outputBytes = formatted.toUtf8() + '\n';
    ::OutputDebugStringA(outputBytes.data());
#endif
}

void IDOSMSVCTarget::flush()
{
}
