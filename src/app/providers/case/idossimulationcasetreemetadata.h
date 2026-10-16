#ifndef IDOS_SIMULATIONCASE_TREE_METADATA_H
#define IDOS_SIMULATIONCASE_TREE_METADATA_H

#include "idos_app.h"
#include "providers/idostreeitemprovidermetadata.h"

class IDOSSimulationCaseTreeProvider;
class IDOSTreeItemProviderRegistry;

class APP_EXPORT IDOSSimulationCaseTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    explicit IDOSSimulationCaseTreeMetadata(IDOSTreeItemProviderRegistry* registry);
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* createProvider(IDOSDataObject* data) const override;

  private:
    IDOSTreeItemProviderRegistry* m_registry;
};

#endif
