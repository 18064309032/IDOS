#ifndef IDOS_SIMULATIONCASE_TREE_METADATA_H
#define IDOS_SIMULATIONCASE_TREE_METADATA_H

#include "idos_app.h"
#include "tree/idostreeitemprovidermetadata.h"

class IDOSSimulationCaseTreeProvider;
class IDOSTreeItemProviderRegistry;

class APP_EXPORT IDOSSimulationCaseTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    explicit IDOSSimulationCaseTreeMetadata(IDOSTreeItemProviderRegistry* registry);
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;

  private:
    IDOSTreeItemProviderRegistry* m_registry;
};

#endif
