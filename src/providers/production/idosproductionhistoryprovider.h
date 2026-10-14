#ifndef IDOS_PRODUCTION_HISTORY_PROVIDER_H
#define IDOS_PRODUCTION_HISTORY_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSProductionHistoryProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;

};

#endif // IDOS_PRODUCTION_HISTORY_PROVIDER_H
