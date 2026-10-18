#ifndef IDOS_SIMULATION_SUMMARY_RESULTS_METADATA_H
#define IDOS_SIMULATION_SUMMARY_RESULTS_METADATA_H


#include "data/idosdatametadata.h"

class CORE_EXPORT IDOSSimulationSummaryResultsMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_SIMULATION_SUMMARY_RESULTS_METADATA_H
