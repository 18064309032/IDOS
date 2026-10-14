#ifndef IDOS_MICROSEISMIC_PROVIDER_H
#define IDOS_MICROSEISMIC_PROVIDER_H

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class PROVIDERS_EXPORT IDOSMicroseismicProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_MICROSEISMIC_PROVIDER_H
