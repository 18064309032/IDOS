#include "idosdataprovider.h"

QString IDOSDataProvider::lastError() const
{
    return m_lastError;
}

void IDOSDataProvider::setLastError(const QString& msg)
{
    m_lastError = msg;
}

IDOSDataProvider::~IDOSDataProvider()
{
}
