#ifndef IDOSSIMULATIONCASEMETADATA_H
#define IDOSSIMULATIONCASEMETADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSSimulationCaseMetadata : public IDOSDataMetadata
{
  public:
    IDOSSimulationCaseMetadata();
    ~IDOSSimulationCaseMetadata() override;

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif
