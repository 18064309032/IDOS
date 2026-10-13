#ifndef IDOS_WELL_CHECKSHOT_PROVIDER_H
#define IDOS_WELL_CHECKSHOT_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSWellCheckshotProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
    bool canRead(const QString& filePath) const;
};

#endif
