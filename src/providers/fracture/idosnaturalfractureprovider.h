#ifndef IDOS_NATURAL_FRACTURE_PROVIDER_H
#define IDOS_NATURAL_FRACTURE_PROVIDER_H

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class PROVIDERS_EXPORT IDOSNaturalFractureProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_NATURAL_FRACTURE_PROVIDER_H
