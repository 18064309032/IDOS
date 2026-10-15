#ifndef IDOS_SIMULATION_SUMMARY_PROVIDER_H
#define IDOS_SIMULATION_SUMMARY_PROVIDER_H


#include "idos_providers.h"
#include "providers/idosdataprovider.h"


class PROVIDERS_EXPORT IDOSSimulationSummaryProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_SIMULATION_SUMMARY_PROVIDER_H
