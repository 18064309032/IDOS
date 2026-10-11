#ifndef IDOS_SIMULATIONCASE_TREE_METADATA_H
#define IDOS_SIMULATIONCASE_TREE_METADATA_H

#include "idos_app.h"
#include "idostreemetadata.h"

class IDOSSimulationCaseTreeProvider;
class IDOSTreeRegistry;

class APP_EXPORT IDOSSimulationCaseTreeMetadata : public IDOSTreeMetadata
{
  public:
    explicit IDOSSimulationCaseTreeMetadata(IDOSTreeRegistry* registry);
    QString typeId() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;

  private:
    IDOSTreeRegistry* m_registry;
};

#endif
